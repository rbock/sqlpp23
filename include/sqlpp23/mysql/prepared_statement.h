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

#include <sqlpp23/core/chrono.h>
#include <sqlpp23/mysql/database/exception.h>
#include <sqlpp23/mysql/database/connection_config.h>
#include <sqlpp23/mysql/sqlpp_mysql.h>

namespace sqlpp::mysql {

namespace detail {

struct wrapped_bool {
  my_bool value;

  wrapped_bool() : value{false} {}
  wrapped_bool(bool v) : value{v} {}
  wrapped_bool(const wrapped_bool&) = default;
  wrapped_bool(wrapped_bool&&) = default;
  wrapped_bool& operator=(const wrapped_bool&) = default;
  wrapped_bool& operator=(wrapped_bool&&) = default;
  ~wrapped_bool() = default;
};

}  // namespace detail

class connection_base;

class prepared_statement_t {
  friend ::sqlpp::mysql::connection_base;

  std::shared_ptr<MYSQL_STMT> mysql_stmt;
  std::vector<MYSQL_BIND> stmt_params;
  std::vector<MYSQL_TIME> stmt_date_time_param_buffer;
  std::vector<detail::wrapped_bool>
      stmt_param_is_null;  // my_bool is bool after 8.0, and vector<bool> is bad
  const connection_config* _config;

 public:
  prepared_statement_t() = delete;
  prepared_statement_t(MYSQL* connection,
                              const std::string& statement,
                              size_t no_of_parameters,
                              const connection_config* config)
      : mysql_stmt{mysql_stmt_init(connection), mysql_stmt_close},
        stmt_params(no_of_parameters,
                    MYSQL_BIND{}),  // ()-init for correct constructor
        stmt_date_time_param_buffer(
            no_of_parameters,
            MYSQL_TIME{}),  // ()-init for correct constructor
        stmt_param_is_null(no_of_parameters,
                           false),  // ()-init for correct constructor
        _config{config} {
    if (mysql_stmt_prepare(native_handle().get(), statement.data(),
                           statement.size())) {
      throw exception{mysql_error(connection), mysql_errno(connection)};
    }
    if constexpr (debug_enabled) {
        debug().log(
            log_category::statement,
            "Constructed prepared_statement, using handle at {}",
            std::hash<void*>{}(native_handle().get()));
    }
  }

  prepared_statement_t(const prepared_statement_t&) = delete;
  prepared_statement_t(prepared_statement_t&& rhs) = default;
  prepared_statement_t& operator=(const prepared_statement_t&) = delete;
  prepared_statement_t& operator=(prepared_statement_t&&) = default;
  ~prepared_statement_t() = default;

  std::shared_ptr<MYSQL_STMT> native_handle() const { return mysql_stmt; }
  std::vector<MYSQL_BIND> parameters() {return stmt_params; }

  const debug_logger& debug() { return _config->debug; }

  bool operator==(const prepared_statement_t& rhs) const {
    return native_handle() == rhs.native_handle();
  }

  void _pre_bind();

  void _bind_parameter(size_t index, const bool& value) {
    if constexpr (debug_enabled) {
      debug().log(log_category::parameter,
                           "binding boolean parameter {} at index {}", value,
                           index);
    }

    stmt_param_is_null[index] = false;
    MYSQL_BIND& param{stmt_params[index]};
    param.buffer_type = MYSQL_TYPE_TINY;
    param.buffer = const_cast<bool*>(&value);
    param.buffer_length = sizeof(value);
    param.length = &param.buffer_length;
    param.is_null = &stmt_param_is_null[index].value;
    param.is_unsigned = false;
    param.error = nullptr;
  }

  void _bind_parameter(size_t index, const int64_t& value) {
    if constexpr (debug_enabled) {
      debug().log(log_category::parameter,
                           "binding integral parameter {} at index {}", value,
                           index);
    }

    stmt_param_is_null[index] = false;
    MYSQL_BIND& param{stmt_params[index]};
    param.buffer_type = MYSQL_TYPE_LONGLONG;
    param.buffer = const_cast<int64_t*>(&value);
    param.buffer_length = sizeof(value);
    param.length = &param.buffer_length;
    param.is_null = &stmt_param_is_null[index].value;
    param.is_unsigned = false;
    param.error = nullptr;
  }

  void _bind_parameter(size_t index, const uint64_t& value) {
    if constexpr (debug_enabled) {
      debug().log(log_category::parameter,
                           "binding unsigned integral parameter {} at index {}",
                           value, index);
    }

    stmt_param_is_null[index] = false;
    MYSQL_BIND& param{stmt_params[index]};
    param.buffer_type = MYSQL_TYPE_LONGLONG;
    param.buffer = const_cast<uint64_t*>(&value);
    param.buffer_length = sizeof(value);
    param.length = &param.buffer_length;
    param.is_null = &stmt_param_is_null[index].value;
    param.is_unsigned = true;
    param.error = nullptr;
  }

  void _bind_parameter(size_t index, const double& value) {
    if constexpr (debug_enabled) {
      debug().log(log_category::parameter,
                           "binding floating_point parameter {} at index {}",
                           value, index);
    }

    stmt_param_is_null[index] = false;
    MYSQL_BIND& param{stmt_params[index]};
    param.buffer_type = MYSQL_TYPE_DOUBLE;
    param.buffer = const_cast<double*>(&value);
    param.buffer_length = sizeof(value);
    param.length = &param.buffer_length;
    param.is_null = &stmt_param_is_null[index].value;
    param.is_unsigned = false;
    param.error = nullptr;
  }

  void _bind_parameter(size_t index, const std::string_view& value) {
    if constexpr (debug_enabled) {
      debug().log(log_category::parameter,
                           "binding text parameter {} at index {}", value,
                           index);
    }

    stmt_param_is_null[index] = false;
    MYSQL_BIND& param{stmt_params[index]};
    param.buffer_type = MYSQL_TYPE_STRING;
    param.buffer = const_cast<char*>(value.data());
    param.buffer_length = value.size();
    param.length = &param.buffer_length;
    param.is_null = &stmt_param_is_null[index].value;
    param.is_unsigned = false;
    param.error = nullptr;
  }

  void _bind_parameter(size_t index, const std::chrono::sys_days& value) {
    if constexpr (debug_enabled) {
      debug().log(log_category::parameter,
                           "binding date parameter {} at index {}", value,
                           index);
    }

    auto& bound_time = stmt_date_time_param_buffer[index];
    const auto ymd = std::chrono::year_month_day{value};
    bound_time.year =
        static_cast<unsigned>(std::abs(static_cast<int>(ymd.year())));
    bound_time.month = static_cast<unsigned>(ymd.month());
    bound_time.day = static_cast<unsigned>(ymd.day());
    bound_time.hour = 0u;
    bound_time.minute = 0u;
    bound_time.second = 0u;
    bound_time.second_part = 0u;
    if constexpr (debug_enabled) {
      debug().log(
          log_category::parameter, "bound values: {}-{}-{}T{}:{}:{}.{}",
          bound_time.year, bound_time.month, bound_time.day, bound_time.hour,
          bound_time.minute, bound_time.second, bound_time.second_part);
    }

    stmt_param_is_null[index] = false;
    MYSQL_BIND& param{stmt_params[index]};
    param.buffer_type = MYSQL_TYPE_DATE;
    param.buffer = &bound_time;
    param.buffer_length = sizeof(MYSQL_TIME);
    param.length = &param.buffer_length;
    param.is_null = &stmt_param_is_null[index].value;
    param.is_unsigned = false;
    param.error = nullptr;
  }

  void _bind_parameter(size_t index,
                       const ::sqlpp::chrono::sys_microseconds& value) {
    if constexpr (debug_enabled) {
      debug().log(log_category::parameter,
                           "binding date_time parameter {} at index {}", value,
                           index);
    }

    auto& bound_time = stmt_date_time_param_buffer[index];
    const auto dp = std::chrono::floor<std::chrono::days>(value);
    const auto time = std::chrono::hh_mm_ss(
        std::chrono::floor<::std::chrono::microseconds>(value - dp));
    const auto ymd = std::chrono::year_month_day{dp};
    bound_time.year =
        static_cast<unsigned>(std::abs(static_cast<int>(ymd.year())));
    bound_time.month = static_cast<unsigned>(ymd.month());
    bound_time.day = static_cast<unsigned>(ymd.day());
    bound_time.hour = static_cast<unsigned>(time.hours().count());
    bound_time.minute = static_cast<unsigned>(time.minutes().count());
    bound_time.second = static_cast<unsigned>(time.seconds().count());
    bound_time.second_part =
        static_cast<unsigned long>(time.subseconds().count());
    if constexpr (debug_enabled) {
      debug().log(
          log_category::parameter, "bound values: {}-{}-{}T{}:{}:{}.{}",
          bound_time.year, bound_time.month, bound_time.day, bound_time.hour,
          bound_time.minute, bound_time.second, bound_time.second_part);
    }

    stmt_param_is_null[index] = false;
    MYSQL_BIND& param{stmt_params[index]};
    param.buffer_type = MYSQL_TYPE_DATETIME;
    param.buffer = &bound_time;
    param.buffer_length = sizeof(MYSQL_TIME);
    param.length = &param.buffer_length;
    param.is_null = &stmt_param_is_null[index].value;
    param.is_unsigned = false;
    param.error = nullptr;
  }

  void _bind_parameter(size_t index, const ::std::chrono::microseconds& value) {
    if constexpr (debug_enabled) {
      debug().log(log_category::parameter,
                           "binding time parameter {} at index {}",
                           value, index);
    }

    auto& bound_time = stmt_date_time_param_buffer[index];
    const auto dp = std::chrono::floor<std::chrono::days>(value);
    const auto time = std::chrono::hh_mm_ss(
        std::chrono::floor<::std::chrono::microseconds>(value - dp));
    bound_time.year = 0u;
    bound_time.month = 0u;
    bound_time.day = 0u;
    bound_time.hour = static_cast<unsigned>(time.hours().count());
    bound_time.minute = static_cast<unsigned>(time.minutes().count());
    bound_time.second = static_cast<unsigned>(time.seconds().count());
    bound_time.second_part =
        static_cast<unsigned long>(time.subseconds().count());
    if constexpr (debug_enabled) {
      debug().log(
          log_category::parameter, "bound values: {}-{}-{}T{}:{}:{}.{}",
          bound_time.year, bound_time.month, bound_time.day, bound_time.hour,
          bound_time.minute, bound_time.second, bound_time.second_part);
    }

    stmt_param_is_null[index] = false;
    MYSQL_BIND& param{stmt_params[index]};
    param.buffer_type = MYSQL_TYPE_TIME;
    param.buffer = &bound_time;
    param.buffer_length = sizeof(MYSQL_TIME);
    param.length = &param.buffer_length;
    param.is_null = &stmt_param_is_null[index].value;
    param.is_unsigned = false;
    param.error = nullptr;
  }

  template <typename Parameter>
  void _bind_parameter(size_t index,
                       const std::optional<Parameter>& parameter) {
    if (parameter.has_value()) {
      _bind_parameter(index, parameter.value());
      return;
    }

    if constexpr (debug_enabled) {
      debug().log(log_category::parameter, "binding NULL parameter {}",
                           index);
    }

    stmt_param_is_null[index] = true;
    MYSQL_BIND& param{stmt_params[index]};
    param.buffer_type = MYSQL_TYPE_TIME;
    param.buffer = &stmt_date_time_param_buffer[index];
    param.buffer_length = sizeof(MYSQL_TIME);
    param.length = &param.buffer_length;
    param.is_null = &stmt_param_is_null[index].value;
    param.is_unsigned = false;
    param.error = nullptr;
  }
};
}  // namespace sqlpp::mysql
