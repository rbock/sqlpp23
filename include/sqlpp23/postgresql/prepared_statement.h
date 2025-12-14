#pragma once

/**
 * Copyright © 2014-2015, Matthijs Möhlmann
 * Copyright © 2021-2021, Roland Bock
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

#include <string>

#include <libpq-fe.h>

#include <sqlpp23/core/chrono.h>
#include <sqlpp23/core/debug_logger.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/postgresql/database/connection_handle.h>
#include <sqlpp23/postgresql/database/serializer_context.h>
#include <sqlpp23/postgresql/pg_result.h>
#include <sqlpp23/postgresql/to_sql_string.h>

namespace sqlpp::postgresql {
// Forward declaration
class connection_base;

// Detail namespace
namespace detail {
struct prepared_statement_handle_t;
}

class prepared_statement_t {
 private:
  friend class sqlpp::postgresql::connection_base;

  ::PGconn* _connection;
  std::string _name;

   // Parameters
  std::vector<bool> _stmt_null_parameters;
  std::vector<std::string> _stmt_parameters;

  const connection_config* _config;

 public:
  prepared_statement_t() = delete;
  // ctor
  prepared_statement_t(::PGconn* connection,
                       const std::string& statement,
                       std::string name,
                       size_t no_of_parameters,
                       const connection_config* config)
      : _connection{connection},_name{std::move(name)},
        _stmt_null_parameters(no_of_parameters, false),
        _stmt_parameters(no_of_parameters, std::string{}),
        _config{config} {
    if constexpr (debug_enabled) {
      config->debug.log(log_category::statement,
                        "constructing prepared_statement, using handle at: {}",
                        std::hash<void*>{}(_connection));
    }

    // This will throw if preparation fails
    pg_result_t{PQprepare(_connection, _name.c_str(), statement.c_str(),
                          /*nParams*/ 0, /*paramTypes*/ nullptr)};
  }

  prepared_statement_t(const prepared_statement_t&) = delete;
  prepared_statement_t(prepared_statement_t&&) = default;
  prepared_statement_t& operator=(const prepared_statement_t&) = delete;
  prepared_statement_t& operator=(prepared_statement_t&&) = default;
  ~prepared_statement_t() {
    // PQclosePrepared is not available in all versions
    // See https://www.postgresql.org/docs/16/libpq-exec.html
    std::string cmd = "DEALLOCATE \"" + _name + "\"";
    PGresult* result = PQexec(_connection, cmd.c_str());
    PQclear(result);
  }

  bool operator==(const prepared_statement_t& rhs) {
    return (this->_name == rhs._name);
  }

  const std::string& name() const { return _name; }

  pg_result_t execute() {
    const size_t size = _stmt_parameters.size();

    std::vector<const char*> values;
    values.reserve(size);
    for (size_t i = 0u; i < size; i++) {
      values.push_back(_stmt_null_parameters[i] ? nullptr
                                                : _stmt_parameters[i].c_str());
    }

    // Execute prepared statement with the parameters.
    return pg_result_t{PQexecPrepared(_connection, /*stmtName*/ _name.data(),
                                 /*nParams*/ static_cast<int>(size),
                                 /*paramValues*/ values.data(),
                                 /*paramLengths*/ nullptr,
                                 /*paramFormats*/ nullptr, /*resultFormat*/ 0)};
  }

  void _bind_parameter(size_t index, const bool& value) {
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::parameter,
                           "binding boolean parameter {} at index {}", value,
                           index);
    }

    _stmt_null_parameters[index] = false;
    if (value) {
      _stmt_parameters[index] = "t";
    } else {
      _stmt_parameters[index] = "f";
    }
  }

  void _bind_parameter(size_t index, const double& value) {
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::parameter,
                           "binding floating_point parameter {} at index {}",
                           value, index);
    }

    _stmt_null_parameters[index] = false;
    context_t context{nullptr};
    using sqlpp::to_sql_string;
    _stmt_parameters[index] = to_sql_string(context, value);
  }

  void _bind_parameter(size_t index, const int64_t& value) {
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::parameter,
                           "binding integral parameter {} at index {}", value,
                           index);
    }

    // Assign values
    _stmt_null_parameters[index] = false;
    _stmt_parameters[index] = std::to_string(value);
  }

  void _bind_parameter(size_t index, const std::string& value) {
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::parameter,
                           "binding text parameter {} at index {}", value,
                           index);
    }

    // Assign values
    _stmt_null_parameters[index] = false;
    _stmt_parameters[index] = value;
  }

  void _bind_parameter(size_t index, const std::chrono::sys_days& value) {
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::parameter,
                           "binding date parameter {} at index {} ", value,
                           index);
    }
    _stmt_null_parameters[index] = false;
    const auto ymd = std::chrono::year_month_day{value};
    _stmt_parameters[index] = std::format("{}", ymd);

    if constexpr (debug_enabled) {
      _config->debug.log(log_category::parameter,
                           "binding date parameter string: {}",
                           _stmt_parameters[index]);
    }
  }

  void _bind_parameter(size_t index, const ::std::chrono::microseconds& value) {
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::parameter,
                           "binding time parameter {} at index {}", value,
                           index);
    }
    _stmt_null_parameters[index] = false;
    const auto dp = std::chrono::floor<std::chrono::days>(value);
    const auto time = std::chrono::hh_mm_ss(
        std::chrono::floor<::std::chrono::microseconds>(value - dp));

    // Timezone handling - always treat the local value as UTC.
    _stmt_parameters[index] = std::format("{}+00", time);
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::parameter,
                           "binding time parameter string: {}",
                           _stmt_parameters[index]);
    }
  }

  void _bind_parameter(size_t index,
                       const ::sqlpp::chrono::sys_microseconds& value) {
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::parameter,
                           "binding date_time parameter at index {}", index);
    }
    _stmt_null_parameters[index] = false;
    const auto dp = std::chrono::floor<std::chrono::days>(value);
    const auto time = std::chrono::hh_mm_ss(
        std::chrono::floor<::std::chrono::microseconds>(value - dp));
    const auto ymd = std::chrono::year_month_day{dp};

    // Timezone handling - always treat the local value as UTC.
    _stmt_parameters[index] = std::format("{} {}+00", ymd, time);
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::parameter,
                           "binding date_time parameter string: {}",
                           _stmt_parameters[index]);
    }
  }

  void _bind_parameter(size_t index, const std::vector<unsigned char>& value) {
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::parameter,
                           "binding blob parameter at index {}", index);
    }
    _stmt_null_parameters[index] = false;
    constexpr char hex_chars[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    auto param = std::string(value.size() * 2 + 2,
                             '\0');  // ()-init for correct constructor
    param[0] = '\\';
    param[1] = 'x';
    auto i = size_t{1};
    for (const auto c : value) {
      param[++i] = hex_chars[c >> 4];
      param[++i] = hex_chars[c & 0x0F];
    }
    _stmt_parameters[index] = std::move(param);
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::parameter,
                           "binding blob parameter string (up to 100 "
                           "chars): {}",
                           _stmt_parameters[index].substr(0, 100));
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
      _config->debug.log(log_category::parameter,
                           "binding NULL parameter at index {}", index);
    }
    _stmt_null_parameters[index] = true;
  }
};
}  // namespace sqlpp::postgresql
