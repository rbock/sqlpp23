#pragma once

/*
 * Copyright (c) 2026
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
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

#include <duckdb.h>

#include <algorithm>
#include <chrono>
#include <cctype>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <sqlpp23/core/chrono.h>
#include <sqlpp23/duck_db/database/connection_config.h>
#include <sqlpp23/duck_db/database/exception.h>

namespace sqlpp::duck_db {
// Forward declaration
class connection_base;

class prepared_statement_t {
  friend class ::sqlpp::duck_db::connection_base;
  struct statement_deleter {
    void operator()(::duckdb_prepared_statement* statement) const noexcept {
      if (statement) {
        ::duckdb_destroy_prepare(statement);
        delete statement;
      }
    }
  };

  ::duckdb_connection _connection = nullptr;
  std::unique_ptr<::duckdb_prepared_statement, statement_deleter> _statement;
  const connection_config* config = nullptr;

 public:
  prepared_statement_t() = delete;
  prepared_statement_t(::duckdb_connection connection,
                       std::string_view statement,
                       const connection_config* config_)
      : _connection{connection}, config{config_} {
    if constexpr (debug_enabled) {
      config->debug.log(log_category::statement, "DuckDB: preparing '{}'",
                        statement);
    }
    if (connection) {
      if constexpr (debug_enabled) {
        config->debug.log(
            log_category::statement,
            "DuckDB: constructing prepared_statement, using handle at {}",
            std::hash<void*>{}(connection));
      }
    }

    // DuckDB prepares exactly one statement. Extract the statements first so
    // that multi-statement input is rejected with a useful message instead of an
    // opaque prepare error.
    const auto statement_text = std::string(statement);
    ::duckdb_extracted_statements extracted = nullptr;
    const auto statement_count = ::duckdb_extract_statements(
        connection, statement_text.c_str(), &extracted);
    const auto* extraction_error = ::duckdb_extract_statements_error(extracted);
    const std::string extraction_error_message =
        extraction_error ? extraction_error : "";
    ::duckdb_destroy_extracted(&extracted);

    if (not extraction_error_message.empty()) {
      throw exception{extraction_error_message, static_cast<int>(DuckDBError)};
    }
    if (statement_count == 0) {
      throw sqlpp::exception{"DuckDB connector: empty statement: >>" +
                             statement_text + "<<\n"};
    }
    if (statement_count != 1) {
      throw sqlpp::exception{
          "DuckDB connector: Cannot execute multi-statements: >>" +
          statement_text + "<<\n"};
    }

    auto prepared =
        std::unique_ptr<::duckdb_prepared_statement, statement_deleter>{
            new ::duckdb_prepared_statement{nullptr}};
    const auto trimmed = trim_trailing_space(statement);
    const auto rc = ::duckdb_prepare(connection, std::string(trimmed).c_str(),
                                     prepared.get());

    if (rc != DuckDBSuccess) {
      const auto* error = ::duckdb_prepare_error(*prepared);
      throw exception{error ? error : "DuckDB: cannot prepare statement",
                      static_cast<int>(rc)};
    }

    _statement = std::move(prepared);
  }

  prepared_statement_t(const prepared_statement_t&) = delete;
  prepared_statement_t(prepared_statement_t&& rhs) = default;
  prepared_statement_t& operator=(const prepared_statement_t&) = delete;
  prepared_statement_t& operator=(prepared_statement_t&& rhs) = default;
  ~prepared_statement_t() = default;

  bool operator==(const prepared_statement_t& rhs) const {
    return _statement == rhs._statement;
  }

  ::duckdb_prepared_statement native_handle() const { return *_statement; }
  const debug_logger& debug() const { return config->debug; }

  // DuckDB statements can be re-executed after rebinding, no reset is required.
  // Clearing the bindings makes the behaviour explicit and matches the sqlite3
  // connector's _reset().
  void _reset() {
    if constexpr (debug_enabled) {
      config->debug.log(log_category::statement,
                        "DuckDB debug: resetting prepared statement");
    }
    if (::duckdb_clear_bindings(*_statement) != DuckDBSuccess) {
      throw exception{"DuckDB: cannot clear parameter bindings",
                      static_cast<int>(DuckDBError)};
    }
  }

  size_t parameter_count() const { return ::duckdb_nparams(*_statement); }

  void bind_parameter(size_t parameter_index, const bool& value) {
    check_binding(::duckdb_bind_boolean(*_statement, index(parameter_index),
                                        value),
                  parameter_index);
  }

  void bind_parameter(size_t parameter_index, const double& value) {
    check_binding(
        ::duckdb_bind_double(*_statement, index(parameter_index), value),
        parameter_index);
  }

  void bind_parameter(size_t parameter_index, const int64_t& value) {
    check_binding(
        ::duckdb_bind_int64(*_statement, index(parameter_index), value),
        parameter_index);
  }

  void bind_parameter(size_t parameter_index, const uint64_t& value) {
    check_binding(
        ::duckdb_bind_uint64(*_statement, index(parameter_index), value),
        parameter_index);
  }

  void bind_parameter(size_t parameter_index, const std::string& value) {
    check_binding(::duckdb_bind_varchar_length(*_statement,
                                              index(parameter_index),
                                              value.data(),
                                              static_cast<::idx_t>(value.size())),
                  parameter_index);
  }

  void bind_parameter(size_t parameter_index,
                      const std::chrono::microseconds& value) {
    ::duckdb_time time{};
    time.micros = static_cast<int64_t>(value.count());
    check_binding(
        ::duckdb_bind_time(*_statement, index(parameter_index), time),
        parameter_index);
  }

  void bind_parameter(size_t parameter_index, const std::chrono::sys_days& value) {
    ::duckdb_date date{};
    date.days = static_cast<int32_t>(
        std::chrono::duration_cast<std::chrono::days>(value.time_since_epoch())
            .count());
    check_binding(
        ::duckdb_bind_date(*_statement, index(parameter_index), date),
        parameter_index);
  }

  void bind_parameter(size_t parameter_index,
                      const ::sqlpp::chrono::sys_microseconds& value) {
    ::duckdb_timestamp timestamp{};
    timestamp.micros =
        std::chrono::duration_cast<std::chrono::microseconds>(
            value.time_since_epoch())
            .count();
    check_binding(
        ::duckdb_bind_timestamp(*_statement, index(parameter_index), timestamp),
        parameter_index);
  }

  void bind_parameter(size_t parameter_index, const std::vector<uint8_t>& value) {
    check_binding(::duckdb_bind_blob(*_statement, index(parameter_index),
                                    value.data(),
                                    static_cast<::idx_t>(value.size())),
                  parameter_index);
  }

  void bind_null(size_t parameter_index) {
    if constexpr (debug_enabled) {
      config->debug.log(
          log_category::parameter,
          "DuckDB debug: binding NULL parameter at parameter_index {}",
          parameter_index);
    }

    check_binding(::duckdb_bind_null(*_statement, index(parameter_index)),
                  parameter_index);
  }

 private:
  static std::string_view trim_trailing_space(std::string_view statement) {
    const auto end = std::find_if(statement.rbegin(), statement.rend(),
                                  [](char ch) { return !std::isspace(ch); })
                         .base();
    return statement.substr(0, static_cast<size_t>(end - statement.begin()));
  }

  static ::idx_t index(size_t parameter_index) {
    // DuckDB binds parameters by their 1-based position.
    return static_cast<::idx_t>(parameter_index + 1);
  }

  void check_binding(::duckdb_state rc, size_t parameter_index) const {
    if (rc != DuckDBSuccess) {
      const auto* error = ::duckdb_prepare_error(*_statement);
      throw exception{std::string{error ? error
                                        : "DuckDB: cannot bind parameter"} +
                          " (parameter_index " +
                          std::to_string(parameter_index) + ")",
                      static_cast<int>(rc)};
    }
  }
};

inline void bind_parameter(prepared_statement_t& statement,
                           size_t parameter_index,
                           const bool& value) {
  if constexpr (debug_enabled) {
    statement.debug().log(
        log_category::parameter,
        "DuckDB debug: binding boolean parameter {} at parameter_index {}", value,
        parameter_index);
  }
  statement.bind_parameter(parameter_index, value);
}

inline void bind_parameter(prepared_statement_t& statement,
                           size_t parameter_index,
                           const double& value) {
  if constexpr (debug_enabled) {
    statement.debug().log(log_category::parameter,
                          "DuckDB debug: binding floating_point parameter {} "
                          "at parameter_index {}",
                          value, parameter_index);
  }
  statement.bind_parameter(parameter_index, value);
}

inline void bind_parameter(prepared_statement_t& statement,
                           size_t parameter_index,
                           const int64_t& value) {
  if constexpr (debug_enabled) {
    statement.debug().log(
        log_category::parameter,
        "DuckDB debug: binding integral parameter {} at parameter_index {}",
        value, parameter_index);
  }
  statement.bind_parameter(parameter_index, value);
}

inline void bind_parameter(prepared_statement_t& statement,
                           size_t parameter_index,
                           const uint64_t& value) {
  if constexpr (debug_enabled) {
    statement.debug().log(log_category::parameter,
                          "DuckDB debug: binding unsigned integral parameter "
                          "{} at parameter_index {}",
                          value, parameter_index);
  }
  statement.bind_parameter(parameter_index, value);
}

inline void bind_parameter(prepared_statement_t& statement,
                           size_t parameter_index,
                           const std::string& value) {
  if constexpr (debug_enabled) {
    statement.debug().log(
        log_category::parameter,
        "DuckDB debug: binding text parameter {} at parameter_index {}", value,
        parameter_index);
  }
  statement.bind_parameter(parameter_index, value);
}

inline void bind_parameter(prepared_statement_t& statement,
                           size_t parameter_index,
                           const std::chrono::microseconds& value) {
  if constexpr (debug_enabled) {
    statement.debug().log(
        log_category::parameter,
        "DuckDB debug: binding time of day parameter {} at parameter_index {}",
        value, parameter_index);
  }
  statement.bind_parameter(parameter_index, value);
}

inline void bind_parameter(prepared_statement_t& statement,
                           size_t parameter_index,
                           const std::chrono::sys_days& value) {
  if constexpr (debug_enabled) {
    statement.debug().log(
        log_category::parameter,
        "DuckDB debug: binding date parameter {} at parameter_index {}", value,
        parameter_index);
  }
  statement.bind_parameter(parameter_index, value);
}

inline void bind_parameter(prepared_statement_t& statement,
                           size_t parameter_index,
                           const ::sqlpp::chrono::sys_microseconds& value) {
  if constexpr (debug_enabled) {
    statement.debug().log(
        log_category::parameter,
        "DuckDB debug: binding date_time parameter {} at parameter_index {}",
        value, parameter_index);
  }
  statement.bind_parameter(parameter_index, value);
}

inline void bind_parameter(prepared_statement_t& statement,
                           size_t parameter_index,
                           const std::vector<uint8_t>& value) {
  if constexpr (debug_enabled) {
    statement.debug().log(log_category::parameter,
                          "DuckDB debug: binding blob parameter size of {} at "
                          "parameter_index {}",
                          value.size(), parameter_index);
  }
  statement.bind_parameter(parameter_index, value);
}

}  // namespace sqlpp::duck_db
