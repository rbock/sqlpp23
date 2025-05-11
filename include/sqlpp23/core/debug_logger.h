#pragma once

/*
 * Copyright (c) 2025, Roland Bock
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

#include <functional>
#include <format>
#include <string>

namespace sqlpp {
#ifdef SQLPP23_DISABLE_DEBUG
static constexpr inline bool debug_enabled = false;
#else
static constexpr inline bool debug_enabled = true;
#endif

enum class log_category : int {
  statement,   // Preparation and execution of statements.
  parameter,   // The parameters sent with a prepared query.
  result,      // Result fields and rows.
  connection,  // Other interactions with the connection, e.g. opening, closing.
};

using log_function_t = std::function<void (const std::string&)>;

class debug_logger {
  size_t _categories = 0;
  log_function_t _log_function;

 public:
  debug_logger() = default;
  debug_logger(const std::vector<log_category>& categories, log_function_t log_function)
      : _log_function(std::move(log_function)) {
    for (auto category : categories) {
      _categories |=
          (1 << static_cast<std::underlying_type_t<log_category>>(category));
    }
  }
  debug_logger(const debug_logger&) = default;
  debug_logger(debug_logger&&) = default;
  debug_logger& operator=(const debug_logger&) = default;
  debug_logger& operator=(debug_logger&&) = default;
  ~debug_logger() = default;

  template <typename... Args>
  void log(log_category category,
           std::format_string<Args...> fmt,
           Args&&... args) const {
    const auto category_bit =
        (1 << static_cast<std::underlying_type_t<log_category>>(category));
    if (_categories & category_bit) {
      _log_function(std::format(fmt, std::forward<Args>(args)...));
    }
  }
};

}  // namespace sqlpp
