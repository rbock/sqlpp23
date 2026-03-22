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

#include <cstdlib>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include <sqlpp23/core/chrono.h>
#include <sqlpp23/core/database/exception.h>
#include <sqlpp23/core/detail/parse_date_time.h>
#include <sqlpp23/core/query/result_row.h>
#include <sqlpp23/mock_db/database/connection_config.h>

struct MockRes {
  std::vector<std::vector<std::optional<std::string>>> rows;
};

namespace sqlpp::mock_db {

class text_result_t {
  size_t _row_index = std::numeric_limits<std::size_t>::max();
  MockRes* _mock_res = nullptr;
  const connection_config* _config;

 public:
  text_result_t() = default;
  text_result_t(MockRes* mock_res, const connection_config* config)
      : _mock_res{std::move(mock_res)}, _config{config} {
    if (_invalid())
      throw sqlpp::exception{
          "MySQL: Constructing text_result without valid handle"};

    if constexpr (debug_enabled) {
      _config->debug.log(log_category::result,
                           "Constructing result, using mock result at {}",
                           std::hash<void*>{}(_mock_res));
    }
  }

  text_result_t(const text_result_t&) = delete;
  text_result_t(text_result_t&& rhs) = default;
  text_result_t& operator=(const text_result_t&) = delete;
  text_result_t& operator=(text_result_t&&) = default;
  ~text_result_t() = default;

  bool operator==(const text_result_t& rhs) const {
    return _mock_res == rhs._mock_res;
  }

  size_t size() const {
    return _mock_res ? _mock_res->rows.size() : size_t{};
  }

  template <typename ResultRow>
  void next(ResultRow& result_row) {
    if (_invalid()) {
      sqlpp::detail::result_row_bridge{}.invalidate(result_row);
      return;
    }

    if (this->next_impl()) {
      if (not result_row) {
        sqlpp::detail::result_row_bridge{}.validate(result_row);
      }
      sqlpp::detail::result_row_bridge{}.read_fields(result_row, *this);
    } else {
      if (result_row) {
        sqlpp::detail::result_row_bridge{}.invalidate(result_row);
      }
    }
  }

  bool _invalid() const { return !_mock_res; }

  auto& debug() const { return _config->debug; }
  bool get_is_null(size_t field_index) const {
    return _mock_res->rows[_row_index][field_index].has_value();
  }
  std::string& get_field(size_t index) {
    return _mock_res->rows[_row_index][index].value();
  }

  const std::string& get_field(size_t index) const {
    return _mock_res->rows[_row_index][index].value();
  }

 private:
  bool next_impl() {
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::result,
                           "Accessing next row of mock result at ",
                           std::hash<void*>{}(_mock_res));
    }

    // Next row
    ++_row_index;
    if (_row_index < _mock_res->rows.size()) {
      return true;
    }
    return false;
  }
};

inline void read_field(const text_result_t& result, size_t index, bool& value) {
  value =
      (result.get_field(index)[0] == 't' or result.get_field(index)[0] == '1');
}

inline void read_field(const text_result_t& result,
                       size_t index,
                       double& value) {
  value = std::strtod(result.get_field(index).c_str(), nullptr);
}

inline void read_field(const text_result_t& result,
                       size_t index,
                       int64_t& value) {
  value = std::strtoll(result.get_field(index).c_str(), nullptr, 10);
}

inline void read_field(const text_result_t& result,
                       size_t index,
                       uint64_t& value) {
  value = std::strtoull(result.get_field(index).c_str(), nullptr, 10);
}
inline void read_field(const text_result_t& result,
                       size_t index,
                       std::span<const uint8_t>& value) {
  value = std::span<const uint8_t>(
      reinterpret_cast<const uint8_t*>(result.get_field(index).c_str()),
      result.get_field(index).size());
}

inline void read_field(const text_result_t& result,
                       size_t index,
                       std::string_view& value) {
  value = std::string_view(result.get_field(index).c_str(),
                           result.get_field(index).size());
}

inline void read_field(const text_result_t& result,
                       size_t index,
                       std::chrono::sys_days& value) {
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result, "parsing date result at index: {}",
                       index);
  }

  const char* date_string = result.get_field(index).c_str();
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result, "date string: {}", date_string);
  }

  if (::sqlpp::detail::parse_date(value, date_string) == false) {
    if constexpr (debug_enabled) {
      result.debug().log(log_category::result, "invalid date result: {}",
                         date_string);
    }
  }

  if (*date_string) {
    if constexpr (debug_enabled) {
      result.debug().log(log_category::result,
                         "trailing characters in date result: {}", date_string);
    }
  }
}

inline void read_field(const text_result_t& result,
                       size_t index,
                       ::sqlpp::chrono::sys_microseconds& value) {
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result, "parsing date result at index: {}",
                       index);
  }

  const char* date_time_string = result.get_field(index).c_str();
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result, "date_time string: {}",
                       date_time_string);
  }

  if (::sqlpp::detail::parse_timestamp(value, date_time_string) == false) {
    if constexpr (debug_enabled) {
      result.debug().log(log_category::result, "invalid date_time result: {}",
                         date_time_string);
    }
  }

  if (*date_time_string) {
    if constexpr (debug_enabled) {
      result.debug().log(log_category::result,
                         "trailing characters in date_time result: {}",
                         date_time_string);
    }
  }
}

inline void read_field(const text_result_t& result,
                       size_t index,
                       ::std::chrono::microseconds& value) {
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result,
                       "parsing time of day result at index: {}", index);
  }

  const char* time_string = result.get_field(index).c_str();
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result, "time of day string: {}",
                       time_string);
  }

  if (::sqlpp::detail::parse_time(value, time_string) == false) {
    if constexpr (debug_enabled) {
      result.debug().log(log_category::result, "invalid time result: {}",
                         time_string);
    }
  }

  if (*time_string) {
    if constexpr (debug_enabled) {
      result.debug().log(log_category::result,
                         "trailing characters in time result: {}", time_string);
    }
  }
}

}  // namespace sqlpp::mock_db
