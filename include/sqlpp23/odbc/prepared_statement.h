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

#include <chrono>
#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

#include <sql.h>
#include <sqlext.h>

#include <sqlpp23/core/chrono.h>
#include <sqlpp23/odbc/database/connection_config.h>
#include <sqlpp23/odbc/database/exception.h>
#include <sqlpp23/odbc/detail/diagnostics.h>

namespace sqlpp::odbc {
// Forward declaration
class connection_base;

class prepared_statement_t {
  friend class ::sqlpp::odbc::connection_base;

  // Owned storage for parameter values that have to be converted to ODBC
  // structs and for the length/NULL indicators. The driver reads from these
  // addresses when the statement is executed, so they have to remain stable:
  // the vector is sized once in the constructor and never resized.
  struct parameter_data {
    union {
      unsigned char bool_;
      int64_t int64_;
      uint64_t uint64_;
      double double_;
      SQL_DATE_STRUCT date_;
      SQL_TIME_STRUCT time_;
      SQL_TIMESTAMP_STRUCT timestamp_;
    };
    SQLLEN length_or_indicator;
  };

  std::shared_ptr<void> _statement;  // SQLHSTMT
  const connection_config* _config;
  std::vector<parameter_data> _parameters;

 public:
  prepared_statement_t() = delete;
  prepared_statement_t(std::shared_ptr<void> statement,
                       std::string_view statement_text,
                       size_t parameter_count,
                       const connection_config* config)
      : _statement{std::move(statement)},
        _config{config},
        _parameters(parameter_count) {
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::statement, "ODBC: preparing: '{}'",
                         statement_text);
    }
    detail::throw_on_error(
        SQLPrepare(native_handle(),
                   reinterpret_cast<SQLCHAR*>(
                       const_cast<char*>(statement_text.data())),
                   static_cast<SQLINTEGER>(statement_text.size())),
        "ODBC: could not prepare statement", SQL_HANDLE_STMT, native_handle());
  }

  prepared_statement_t(const prepared_statement_t&) = delete;
  prepared_statement_t(prepared_statement_t&& rhs) = default;
  prepared_statement_t& operator=(const prepared_statement_t&) = delete;
  prepared_statement_t& operator=(prepared_statement_t&&) = default;
  ~prepared_statement_t() = default;

  bool operator==(const prepared_statement_t& rhs) const {
    return _statement == rhs._statement;
  }

  SQLHSTMT native_handle() const { return _statement.get(); }
  const debug_logger& debug() const { return _config->debug; }

  void _reset() {
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::statement,
                         "ODBC: closing cursor of prepared statement");
    }
    // Closes the cursor of a previous execution, if any. SQL_CLOSE is
    // explicitly harmless when no cursor is open.
    SQLFreeStmt(native_handle(), SQL_CLOSE);
  }

  void bind_parameter(size_t parameter_index, const bool& value) {
    auto& data = _parameters[parameter_index];
    data.bool_ = value;
    data.length_or_indicator = 0;
    bind(parameter_index, SQL_C_BIT, SQL_BIT, 1, 0, &data.bool_, 0);
  }

  void bind_parameter(size_t parameter_index, const int64_t& value) {
    auto& data = _parameters[parameter_index];
    data.int64_ = value;
    data.length_or_indicator = 0;
    bind(parameter_index, SQL_C_SBIGINT, SQL_BIGINT, 0, 0, &data.int64_, 0);
  }

  void bind_parameter(size_t parameter_index, const uint64_t& value) {
    auto& data = _parameters[parameter_index];
    data.uint64_ = value;
    data.length_or_indicator = 0;
    bind(parameter_index, SQL_C_UBIGINT, SQL_BIGINT, 0, 0, &data.uint64_, 0);
  }

  void bind_parameter(size_t parameter_index, const double& value) {
    auto& data = _parameters[parameter_index];
    data.double_ = value;
    data.length_or_indicator = 0;
    bind(parameter_index, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &data.double_, 0);
  }

  // Note: The text is not copied. The caller (the prepared statement of the
  // core library) owns it and keeps it alive until execution.
  void bind_parameter(size_t parameter_index, const std::string_view& value) {
    auto& data = _parameters[parameter_index];
    data.length_or_indicator = static_cast<SQLLEN>(value.size());
    bind(parameter_index, SQL_C_CHAR,
         value.size() > 8000 ? SQL_LONGVARCHAR : SQL_VARCHAR,
         value.empty() ? 1 : value.size(), 0, const_cast<char*>(value.data()),
         static_cast<SQLLEN>(value.size()));
  }

  // Note: The blob is not copied, see the note for text above.
  void bind_parameter(size_t parameter_index,
                      const std::vector<uint8_t>& value) {
    auto& data = _parameters[parameter_index];
    data.length_or_indicator = static_cast<SQLLEN>(value.size());
    bind(parameter_index, SQL_C_BINARY,
         value.size() > 8000 ? SQL_LONGVARBINARY : SQL_VARBINARY,
         value.empty() ? 1 : value.size(), 0,
         const_cast<uint8_t*>(value.data()), static_cast<SQLLEN>(value.size()));
  }

  void bind_parameter(size_t parameter_index,
                      const std::chrono::sys_days& value) {
    const std::chrono::year_month_day ymd{value};
    auto& data = _parameters[parameter_index];
    data.date_ = {
        .year = static_cast<SQLSMALLINT>(static_cast<int>(ymd.year())),
        .month = static_cast<SQLUSMALLINT>(static_cast<unsigned>(ymd.month())),
        .day = static_cast<SQLUSMALLINT>(static_cast<unsigned>(ymd.day()))};
    data.length_or_indicator = 0;
    bind(parameter_index, SQL_C_TYPE_DATE, SQL_TYPE_DATE, SQL_DATE_LEN, 0,
         &data.date_, 0);
  }

  // Note: ODBC's SQL_TIME_STRUCT has no sub-second precision, fractional
  // seconds are dropped.
  void bind_parameter(size_t parameter_index,
                      const std::chrono::microseconds& value) {
    const auto hours = std::chrono::floor<std::chrono::hours>(value);
    const auto minutes =
        std::chrono::floor<std::chrono::minutes>(value - hours);
    const auto seconds =
        std::chrono::floor<std::chrono::seconds>(value - hours - minutes);
    auto& data = _parameters[parameter_index];
    data.time_ = {.hour = static_cast<SQLUSMALLINT>(hours.count()),
                  .minute = static_cast<SQLUSMALLINT>(minutes.count()),
                  .second = static_cast<SQLUSMALLINT>(seconds.count())};
    data.length_or_indicator = 0;
    bind(parameter_index, SQL_C_TYPE_TIME, SQL_TYPE_TIME, SQL_TIME_LEN, 0,
         &data.time_, 0);
  }

  void bind_parameter(size_t parameter_index,
                      const ::sqlpp::chrono::sys_microseconds& value) {
    const auto days = std::chrono::floor<std::chrono::days>(value);
    const std::chrono::year_month_day ymd{days};
    const std::chrono::hh_mm_ss time{value - days};
    auto& data = _parameters[parameter_index];
    data.timestamp_ = {
        .year = static_cast<SQLSMALLINT>(static_cast<int>(ymd.year())),
        .month = static_cast<SQLUSMALLINT>(static_cast<unsigned>(ymd.month())),
        .day = static_cast<SQLUSMALLINT>(static_cast<unsigned>(ymd.day())),
        .hour = static_cast<SQLUSMALLINT>(time.hours().count()),
        .minute = static_cast<SQLUSMALLINT>(time.minutes().count()),
        .second = static_cast<SQLUSMALLINT>(time.seconds().count()),
        // Billionths of a second.
        .fraction = static_cast<SQLUINTEGER>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                time.subseconds())
                .count())};
    data.length_or_indicator = 0;
    // Column size 26 and 6 decimal digits: "YYYY-MM-DD hh:mm:ss.ffffff"
    bind(parameter_index, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, 26, 6,
         &data.timestamp_, 0);
  }

  void bind_null(size_t parameter_index) {
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::parameter,
                         "ODBC: binding NULL parameter at index {}",
                         parameter_index);
    }
    auto& data = _parameters[parameter_index];
    data.length_or_indicator = SQL_NULL_DATA;
    bind(parameter_index, SQL_C_DEFAULT, SQL_VARCHAR, 1, 0, nullptr, 0);
  }

 private:
  void bind(size_t parameter_index,
            SQLSMALLINT c_type,
            SQLSMALLINT sql_type,
            SQLULEN column_size,
            SQLSMALLINT decimal_digits,
            void* value,
            SQLLEN buffer_length) {
    detail::throw_on_error(
        SQLBindParameter(native_handle(),
                         static_cast<SQLUSMALLINT>(parameter_index + 1),
                         SQL_PARAM_INPUT, c_type, sql_type, column_size,
                         decimal_digits, value, buffer_length,
                         &_parameters[parameter_index].length_or_indicator),
        "ODBC: could not bind parameter", SQL_HANDLE_STMT, native_handle());
  }
};

inline void bind_parameter(prepared_statement_t& statement,
                           size_t parameter_index,
                           const bool& value) {
  if constexpr (debug_enabled) {
    statement.debug().log(log_category::parameter,
                          "ODBC: binding boolean parameter {} at index {}",
                          value, parameter_index);
  }
  statement.bind_parameter(parameter_index, value);
}

inline void bind_parameter(prepared_statement_t& statement,
                           size_t parameter_index,
                           const int64_t& value) {
  if constexpr (debug_enabled) {
    statement.debug().log(log_category::parameter,
                          "ODBC: binding integral parameter {} at index {}",
                          value, parameter_index);
  }
  statement.bind_parameter(parameter_index, value);
}

inline void bind_parameter(prepared_statement_t& statement,
                           size_t parameter_index,
                           const uint64_t& value) {
  if constexpr (debug_enabled) {
    statement.debug().log(
        log_category::parameter,
        "ODBC: binding unsigned integral parameter {} at index {}", value,
        parameter_index);
  }
  statement.bind_parameter(parameter_index, value);
}

inline void bind_parameter(prepared_statement_t& statement,
                           size_t parameter_index,
                           const double& value) {
  if constexpr (debug_enabled) {
    statement.debug().log(
        log_category::parameter,
        "ODBC: binding floating_point parameter {} at index {}", value,
        parameter_index);
  }
  statement.bind_parameter(parameter_index, value);
}

inline void bind_parameter(prepared_statement_t& statement,
                           size_t parameter_index,
                           const std::string_view& value) {
  if constexpr (debug_enabled) {
    statement.debug().log(log_category::parameter,
                          "ODBC: binding text parameter '{}' at index {}",
                          value, parameter_index);
  }
  statement.bind_parameter(parameter_index, value);
}

inline void bind_parameter(prepared_statement_t& statement,
                           size_t parameter_index,
                           const std::vector<uint8_t>& value) {
  if constexpr (debug_enabled) {
    statement.debug().log(
        log_category::parameter,
        "ODBC: binding blob parameter with {} bytes at index {}", value.size(),
        parameter_index);
  }
  statement.bind_parameter(parameter_index, value);
}

inline void bind_parameter(prepared_statement_t& statement,
                           size_t parameter_index,
                           const std::chrono::sys_days& value) {
  if constexpr (debug_enabled) {
    statement.debug().log(log_category::parameter,
                          "ODBC: binding date parameter {} at index {}", value,
                          parameter_index);
  }
  statement.bind_parameter(parameter_index, value);
}

inline void bind_parameter(prepared_statement_t& statement,
                           size_t parameter_index,
                           const std::chrono::microseconds& value) {
  if constexpr (debug_enabled) {
    statement.debug().log(log_category::parameter,
                          "ODBC: binding time of day parameter {} at index {}",
                          value, parameter_index);
  }
  statement.bind_parameter(parameter_index, value);
}

inline void bind_parameter(prepared_statement_t& statement,
                           size_t parameter_index,
                           const ::sqlpp::chrono::sys_microseconds& value) {
  if constexpr (debug_enabled) {
    statement.debug().log(log_category::parameter,
                          "ODBC: binding timestamp parameter {} at index {}",
                          value, parameter_index);
  }
  statement.bind_parameter(parameter_index, value);
}

}  // namespace sqlpp::odbc
