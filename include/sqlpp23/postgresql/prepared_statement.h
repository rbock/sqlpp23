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

#include <memory>
#include <string>
#include "sqlpp23/core/debug_logger.h"

#include <sqlpp23/core/chrono.h>
#include <sqlpp23/postgresql/database/exception.h>
#include <sqlpp23/postgresql/database/serializer_context.h>
#include <sqlpp23/postgresql/detail/prepared_statement_handle.h>
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

  std::shared_ptr<detail::prepared_statement_handle_t> _handle;

 public:
  prepared_statement_t() = default;

  // ctor
  prepared_statement_t(
      std::shared_ptr<detail::prepared_statement_handle_t>&& handle)
      : _handle{handle} {
    if constexpr (debug_enabled) {
      if (_handle) {
        handle->debug().log(
            log_category::statement,
            "constructing prepared_statement, using handle at: {}",
            std::hash<void*>{}(_handle.get()));
      }
    }
  }

  prepared_statement_t(const prepared_statement_t&) = delete;
  prepared_statement_t(prepared_statement_t&&) = default;
  prepared_statement_t& operator=(const prepared_statement_t&) = delete;
  prepared_statement_t& operator=(prepared_statement_t&&) = default;
  ~prepared_statement_t() = default;

  bool operator==(const prepared_statement_t& rhs) {
    return (this->_handle == rhs._handle);
  }

  void _bind_parameter(size_t index, const bool& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::parameter,
                           "binding boolean parameter {} at index {}", value,
                           index);
    }

    _handle->null_values[index] = false;
    if (value) {
      _handle->param_values[index] = "TRUE";
    } else {
      _handle->param_values[index] = "FALSE";
    }
  }

  void _bind_parameter(size_t index, const double& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::parameter,
                           "binding floating_point parameter {} at index {}",
                           value, index);
    }

    _handle->null_values[index] = false;
    context_t context{nullptr};
    _handle->param_values[index] = to_sql_string(context, value);
  }

  void _bind_parameter(size_t index, const int64_t& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::parameter,
                           "binding integral parameter {} at index {}", value,
                           index);
    }

    // Assign values
    _handle->null_values[index] = false;
    _handle->param_values[index] = std::to_string(value);
  }

  void _bind_parameter(size_t index, const std::string& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::parameter,
                           "binding text parameter {} at index {}", value,
                           index);
    }

    // Assign values
    _handle->null_values[index] = false;
    _handle->param_values[index] = value;
  }

  void _bind_parameter(size_t index, const ::sqlpp::chrono::day_point& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::parameter,
                           "binding date parameter {} at index {} ", value,
                           index);
    }
    _handle->null_values[index] = false;
    const auto ymd = std::chrono::year_month_day{value};
    std::ostringstream os;
    os << ymd;
    _handle->param_values[index] = os.str();

    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::parameter,
                           "binding date parameter string: {}",
                           _handle->param_values[index]);
    }
  }

  void _bind_parameter(size_t index, const ::std::chrono::microseconds& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::parameter,
                           "binding time parameter {} at index {}", value,
                           index);
    }
    _handle->null_values[index] = false;
    const auto dp = std::chrono::floor<std::chrono::days>(value);
    const auto time = std::chrono::hh_mm_ss(
        std::chrono::floor<::std::chrono::microseconds>(value - dp));

    // Timezone handling - always treat the local value as UTC.
    std::ostringstream os;
    os << time << "+00";
    _handle->param_values[index] = os.str();
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::parameter,
                           "binding time parameter string: {}",
                           _handle->param_values[index]);
    }
  }

  void _bind_parameter(size_t index,
                       const ::sqlpp::chrono::microsecond_point& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::parameter,
                           "binding date_time parameter at index {}", index);
    }
    _handle->null_values[index] = false;
    const auto dp = std::chrono::floor<std::chrono::days>(value);
    const auto time = std::chrono::hh_mm_ss(
        std::chrono::floor<::std::chrono::microseconds>(value - dp));
    const auto ymd = std::chrono::year_month_day{dp};

    // Timezone handling - always treat the local value as UTC.
    std::ostringstream os;
    os << ymd << ' ' << time << "+00";
    _handle->param_values[index] = os.str();
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::parameter,
                           "binding date_time parameter string: {}",
                           _handle->param_values[index]);
    }
  }

  void _bind_parameter(size_t index, const std::vector<unsigned char>& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::parameter,
                           "binding blob parameter at index {}", index);
    }
    _handle->null_values[index] = false;
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
    _handle->param_values[index] = std::move(param);
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::parameter,
                           "binding blob parameter string (up to 100 "
                           "chars): {}",
                           _handle->param_values[index].substr(0, 100));
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
      _handle->debug().log(log_category::parameter,
                           "binding NULL parameter at index {}", index);
    }
    _handle->null_values[index] = true;
  }
};
}  // namespace sqlpp::postgresql
