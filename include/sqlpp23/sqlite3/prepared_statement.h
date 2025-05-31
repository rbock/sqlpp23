#pragma once

/*
 * Copyright (c) 2013 - 2015, Roland Bock
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

#include <chrono>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

#ifdef SQLPP_USE_SQLCIPHER
#include <sqlcipher/sqlite3.h>
#else
#include <sqlite3.h>
#endif

#include <sqlpp23/core/chrono.h>
#include <sqlpp23/sqlite3/database/exception.h>
#include <sqlpp23/sqlite3/database/connection_config.h>

namespace sqlpp::sqlite3 {
// Forward declaration
class connection_base;

class prepared_statement_t {
  friend class ::sqlpp::sqlite3::connection_base;
  ::sqlite3* _connection;
  std::shared_ptr<sqlite3_stmt> _sqlite3_statement;
  const connection_config* config;

 public:
  prepared_statement_t() = delete;
  prepared_statement_t(::sqlite3* connection,
                       std::string_view statement,
                       const connection_config* config_)
      : _connection{connection}, _sqlite3_statement{nullptr}, config{config_} {
    if constexpr (debug_enabled) {
      config->debug.log(log_category::statement, "Preparing: '{}'", statement);
    }
    if (connection) {
      config->debug.log(log_category::statement,
                        "Constructing prepared_statement, using handle at {}",
                        std::hash<void*>{}(connection));
    }

    // ignore trailing spaces
    const auto end =
        std::find_if(statement.rbegin(), statement.rend(), [](char ch) {
          return !std::isspace(ch);
        }).base();
    const auto length = end - statement.begin();

    ::sqlite3_stmt* native_handle = nullptr;
    const char* uncompiledTail = nullptr;
    const auto rc = ::sqlite3_prepare_v2(connection, statement.data(),
                                         static_cast<int>(length),
                                         &native_handle, &uncompiledTail);
    _sqlite3_statement =
        std::shared_ptr<::sqlite3_stmt>{native_handle, sqlite3_finalize};

    if (rc != SQLITE_OK) {
      throw exception{std::string(sqlite3_errmsg(connection)), rc};
    }

    if (uncompiledTail != statement.data() + length) {
      throw sqlpp::exception{
          "Sqlite3 connector: Cannot execute multi-statements: >>" +
          std::string(statement) + "<<\n"};
    }
  }
  prepared_statement_t(const prepared_statement_t&) = delete;
  prepared_statement_t(prepared_statement_t&& rhs) = default;
  prepared_statement_t& operator=(const prepared_statement_t&) = delete;
  prepared_statement_t& operator=(prepared_statement_t&&) = default;
  ~prepared_statement_t() = default;

  bool operator==(const prepared_statement_t& rhs) const {
    return _sqlite3_statement == rhs._sqlite3_statement;
  }

  ::sqlite3_stmt* native_handle() { return _sqlite3_statement.get(); }
  const debug_logger& debug() const { return config->debug; }

  void _reset() {
    if constexpr (debug_enabled) {
      config->debug.log(log_category::statement,
                        "Sqlite3 debug: resetting prepared statement");
    }
    sqlite3_reset(_sqlite3_statement.get());
  }

  void _bind_parameter(size_t index, const bool& value) {
    if constexpr (debug_enabled) {
      config->debug.log(
          log_category::parameter,
          "Sqlite3 debug: binding boolean parameter {} at index {}", value,
          index);
    }

    const int rc = sqlite3_bind_int(_sqlite3_statement.get(),
                                    static_cast<int>(index + 1), value);
    if (rc != SQLITE_OK) {
      throw exception{sqlite3_errmsg(_connection), rc};
    }
  }

  void _bind_parameter(size_t index, const double& value) {
    if constexpr (debug_enabled) {
      config->debug.log(
          log_category::parameter,
          "Sqlite3 debug: binding floating_point parameter {} at index {}",
          value, index);
    }

    int rc;
    if (std::isnan(value))
      rc = sqlite3_bind_text(_sqlite3_statement.get(),
                             static_cast<int>(index + 1), "NaN", 3,
                             SQLITE_STATIC);
    else if (std::isinf(value)) {
      if (value > std::numeric_limits<double>::max())
        rc = sqlite3_bind_text(_sqlite3_statement.get(),
                               static_cast<int>(index + 1), "Inf", 3,
                               SQLITE_STATIC);
      else
        rc = sqlite3_bind_text(_sqlite3_statement.get(),
                               static_cast<int>(index + 1), "-Inf", 4,
                               SQLITE_STATIC);
    } else
      rc = sqlite3_bind_double(_sqlite3_statement.get(),
                               static_cast<int>(index + 1), value);
    if (rc != SQLITE_OK) {
      throw exception{sqlite3_errmsg(_connection), rc};
    }
  }

  void _bind_parameter(size_t index, const int64_t& value) {
    if constexpr (debug_enabled) {
      config->debug.log(
          log_category::parameter,
          "Sqlite3 debug: binding integral parameter {} at index {}", value,
          index);
    }

    const int rc = sqlite3_bind_int64(_sqlite3_statement.get(),
                                      static_cast<int>(index + 1), value);
    if (rc != SQLITE_OK) {
      throw exception{sqlite3_errmsg(_connection), rc};
    }
  }

  void _bind_parameter(size_t index, const uint64_t& value) {
    if constexpr (debug_enabled) {
      config->debug.log(
          log_category::parameter,
          "Sqlite3 debug: binding unsigned integral parameter {} at index {}",
          value, index);
    }

    const int rc = sqlite3_bind_int64(_sqlite3_statement.get(),
                                      static_cast<int>(index + 1),
                                      static_cast<int64_t>(value));
    if (rc != SQLITE_OK) {
      throw exception{sqlite3_errmsg(_connection), rc};
    }
  }

  void _bind_parameter(size_t index, const std::string& value) {
    if constexpr (debug_enabled) {
      config->debug.log(log_category::parameter,
                        "Sqlite3 debug: binding text parameter {} at index {}",
                        value, index);
    }

    const int rc = sqlite3_bind_text(
        _sqlite3_statement.get(), static_cast<int>(index + 1), value.data(),
        static_cast<int>(value.size()), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
      throw exception{sqlite3_errmsg(_connection), rc};
    }
  }

  void _bind_parameter(size_t index, const std::chrono::microseconds& value) {
    if constexpr (debug_enabled) {
      config->debug.log(
          log_category::parameter,
          "Sqlite3 debug: binding time of day parameter {} at index {}", value,
          index);
    }

    const auto text = std::format("{0:%H:%M:%S}", value);
    const int rc = sqlite3_bind_text(
        _sqlite3_statement.get(), static_cast<int>(index + 1), text.data(),
        static_cast<int>(text.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
      throw exception{sqlite3_errmsg(_connection), rc};
    }
  }

  void _bind_parameter(size_t index, const std::chrono::sys_days& value) {
    if constexpr (debug_enabled) {
      config->debug.log(log_category::parameter,
                        "Sqlite3 debug: binding date parameter {} at index {}",
                        value, index);
    }

    const auto text = std::format("{0:%Y-%m-%d}", value);
    const int rc = sqlite3_bind_text(
        _sqlite3_statement.get(), static_cast<int>(index + 1), text.data(),
        static_cast<int>(text.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
      throw exception{sqlite3_errmsg(_connection), rc};
    }
  }

  void _bind_parameter(size_t index,
                       const ::sqlpp::chrono::sys_microseconds& value) {
    if constexpr (debug_enabled) {
      config->debug.log(
          log_category::parameter,
          "Sqlite3 debug: binding date_time parameter {} at index {}", value,
          index);
    }

    const auto text = std::format("{0:%Y-%m-%d %H:%M:%S}", value);
    const int rc = sqlite3_bind_text(
        _sqlite3_statement.get(), static_cast<int>(index + 1), text.data(),
        static_cast<int>(text.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
      throw exception{sqlite3_errmsg(_connection), rc};
    }
  }

  void _bind_parameter(size_t index, const std::vector<uint8_t>& value) {
    if constexpr (debug_enabled) {
      config->debug.log(
          log_category::parameter,
          "Sqlite3 debug: binding blob parameter size of {} at index {}",
          value.size(), index);
    }

    const int rc = sqlite3_bind_blob(
        _sqlite3_statement.get(), static_cast<int>(index + 1), value.data(),
        static_cast<int>(value.size()), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
      throw exception{sqlite3_errmsg(_connection), rc};
    }
  }

  template <typename Parameter>
  void _bind_parameter(size_t index,
                       const std::optional<Parameter>& parameter) {
    if (parameter.has_value()) {
      _bind_parameter(index, parameter.value());
      return;
    }

    if constexpr (debug_enabled) {
      config->debug.log(log_category::parameter,
                        "Sqlite3 debug: binding NULL parameter at index {}",
                        index);
    }

    const int rc = sqlite3_bind_null(_sqlite3_statement.get(),
                                     static_cast<int>(index + 1));
    if (rc != SQLITE_OK) {
      throw exception{sqlite3_errmsg(_connection), rc};
    }
  }
};
}  // namespace sqlpp::sqlite3

