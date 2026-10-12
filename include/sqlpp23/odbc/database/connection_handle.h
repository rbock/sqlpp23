#pragma once

/*
 * Copyright (c) 2026, Leander Schulten
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <memory>
#include <type_traits>
#include <variant>

#include <sql.h>
#include <sqlext.h>

#include <sqlpp23/core/debug_logger.h>
#include <sqlpp23/odbc/database/connection_config.h>
#include <sqlpp23/odbc/database/exception.h>
#include <sqlpp23/odbc/detail/diagnostics.h>

namespace sqlpp::odbc::detail {
struct handle_deleter {
  SQLSMALLINT handle_type;
  void operator()(SQLHANDLE handle) const noexcept {
    SQLFreeHandle(handle_type, handle);
  }
};

// SQLHANDLE is void*, so this holds any ODBC handle.
using unique_handle =
    std::unique_ptr<std::remove_pointer_t<SQLHANDLE>, handle_deleter>;

inline unique_handle allocate_handle(SQLSMALLINT handle_type,
                                     SQLHANDLE parent) {
  SQLHANDLE handle{SQL_NULL_HANDLE};
  const auto rc = SQLAllocHandle(handle_type, parent, &handle);
  if (rc != SQL_SUCCESS and rc != SQL_SUCCESS_WITH_INFO) {
    if (parent != SQL_NULL_HANDLE) {
      throw make_exception(
          "ODBC: could not allocate handle",
          handle_type == SQL_HANDLE_DBC ? SQL_HANDLE_ENV : SQL_HANDLE_DBC,
          parent);
    }
    throw exception{"ODBC: could not allocate environment handle"};
  }
  return unique_handle{handle, handle_deleter{handle_type}};
}

struct connection_handle {
  std::shared_ptr<const connection_config> config;
  // Declaration order matters: the connection is freed before the environment.
  unique_handle environment;
  unique_handle connection;

  connection_handle() = default;

  connection_handle(const std::shared_ptr<const connection_config>& conf)
      : config{conf},
        environment{allocate_handle(SQL_HANDLE_ENV, SQL_NULL_HANDLE)} {
    throw_on_error(
        SQLSetEnvAttr(environment.get(), SQL_ATTR_ODBC_VERSION,
                      reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3_80), 0),
        "ODBC: could not request ODBC 3.8", SQL_HANDLE_ENV, environment.get());
    connection = allocate_handle(SQL_HANDLE_DBC, environment.get());

    std::visit([this](const auto& source) { connect(source); }, conf->source);
  }

  connection_handle(const connection_handle&) = delete;
  connection_handle(connection_handle&&) = default;
  connection_handle& operator=(const connection_handle&) = delete;
  connection_handle& operator=(connection_handle&& rhs) {
    if (this != &rhs) {
      disconnect();
      config = std::move(rhs.config);
      connection = std::move(rhs.connection);
      environment = std::move(rhs.environment);
    }
    return *this;
  }

  ~connection_handle() { disconnect(); }

  SQLHDBC native_handle() const { return connection.get(); }

  bool is_connected() const {
    if (not connection) {
      return false;
    }
    // SQL_ATTR_CONNECTION_DEAD reflects the last known state of the
    // connection without a server round trip. The attribute is documented as
    // 32 bit, the wider buffer guards against drivers that write SQLULEN.
    SQLULEN dead{SQL_CD_FALSE};
    const auto rc =
        SQLGetConnectAttr(connection.get(), SQL_ATTR_CONNECTION_DEAD, &dead,
                          SQL_IS_UINTEGER, nullptr);
    if (rc != SQL_SUCCESS and rc != SQL_SUCCESS_WITH_INFO) {
      // The driver cannot tell, assume the connection is still alive.
      return true;
    }
    return dead != SQL_CD_TRUE;
  }

  bool ping_server() const {
    // ODBC has no portable ping. Drivers that implement
    // SQL_ATTR_CONNECTION_DEAD actively are covered by this, for others this
    // is a passive check only.
    return is_connected();
  }

  const debug_logger& debug() const { return config->debug; }

 private:
  void connect(const data_source& source) {
    if constexpr (debug_enabled) {
      debug().log(log_category::connection, "ODBC: connecting to DSN '{}'",
                  source.name);
    }
    throw_on_error(
        SQLConnect(
            connection.get(), to_sqlchar_pointer(source.name),
            static_cast<SQLSMALLINT>(source.name.size()),
            source.username.empty() ? nullptr
                                    : to_sqlchar_pointer(source.username),
            static_cast<SQLSMALLINT>(source.username.size()),
            source.password.empty() ? nullptr
                                    : to_sqlchar_pointer(source.password),
            static_cast<SQLSMALLINT>(source.password.size())),
        "ODBC: could not connect to data source '" + source.name + "'",
        SQL_HANDLE_DBC, connection.get());
  }

  void connect(const connection_string& source) {
    if constexpr (debug_enabled) {
      debug().log(log_category::connection,
                  "ODBC: connecting using a connection string");
    }
    throw_on_error(
        SQLDriverConnect(connection.get(), nullptr,
                         to_sqlchar_pointer(source.value),
                         static_cast<SQLSMALLINT>(source.value.size()), nullptr,
                         0, nullptr, SQL_DRIVER_NOPROMPT),
        "ODBC: could not connect using the connection string", SQL_HANDLE_DBC,
        connection.get());
  }

  void disconnect() noexcept {
    if (connection) {
      SQLDisconnect(connection.get());
    }
  }
};
}  // namespace sqlpp::odbc::detail
