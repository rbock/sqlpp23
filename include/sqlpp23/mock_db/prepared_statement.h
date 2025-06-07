#pragma once

/*
 * Copyright (c) 2025, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
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

#include <cstddef>

#include <sqlpp23/core/chrono.h>
#include <sqlpp23/core/debug_logger.h>
#include <sqlpp23/mock_db/database/connection_config.h>

namespace sqlpp::mock_db {
class prepared_statement_t {
  const connection_config* _config;

  public:
  prepared_statement_t() = delete;
  prepared_statement_t(const connection_config* config) : _config{config} {
    if constexpr (debug_enabled) {
      debug().log(log_category::statement, "Constructed prepared_statement");
    }
  }

  prepared_statement_t(const prepared_statement_t&) = delete;
  prepared_statement_t(prepared_statement_t&& rhs) = default;
  prepared_statement_t& operator=(const prepared_statement_t&) = delete;
  prepared_statement_t& operator=(prepared_statement_t&&) = default;
  ~prepared_statement_t() = default;

  void _bind_parameter(size_t index, const bool& value) {
    if constexpr (debug_enabled) {
      debug().log(log_category::parameter,
                  "binding boolean parameter {} at index {}", value, index);
    }
  }

  void _bind_parameter(size_t index, const int64_t& value) {
    if constexpr (debug_enabled) {
      debug().log(log_category::parameter,
                  "binding integral parameter {} at index {}", value, index);
    }
  }

  void _bind_parameter(size_t index, const uint64_t& value) {
    if constexpr (debug_enabled) {
      debug().log(log_category::parameter,
                  "binding unsigned integral parameter {} at index {}", value, index);
    }
  }

  void _bind_parameter(size_t index, const double& value) {
    if constexpr (debug_enabled) {
      debug().log(log_category::parameter,
                  "binding double parameter {} at index {}", value, index);
    }
  }

  void _bind_parameter(size_t index, const std::string_view& value) {
    if constexpr (debug_enabled) {
      debug().log(log_category::parameter,
                  "binding text parameter {} at index {}", value, index);
    }
  }

  void _bind_parameter(size_t index, const std::chrono::sys_days& value) {
    if constexpr (debug_enabled) {
      debug().log(log_category::parameter,
                  "binding date parameter {} at index {}", value, index);
    }
  }

  void _bind_parameter(size_t index,
                       const ::sqlpp::chrono::sys_microseconds& value) {
    if constexpr (debug_enabled) {
      debug().log(log_category::parameter,
                  "binding date_time parameter {} at index {}", value, index);
    }
  }

  void _bind_parameter(size_t index, const ::std::chrono::microseconds& value) {
    if constexpr (debug_enabled) {
      debug().log(log_category::parameter,
                  "binding time_of_day parameter {} at index {}", value, index);
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
      debug().log(log_category::parameter, "binding NULL parameter {}", index);
    }
  }

  const debug_logger& debug() { return _config->debug; }
};

}  // namespace sqlpp::mock_db
