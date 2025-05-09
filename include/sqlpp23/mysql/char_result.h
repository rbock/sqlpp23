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

#include <optional>
#include <span>
#include <string_view>

#include <sqlpp23/core/chrono.h>
#include <sqlpp23/core/database/exception.h>
#include <sqlpp23/core/detail/parse_date_time.h>
#include <sqlpp23/core/query/result_row.h>
#include <sqlpp23/mysql/char_result_row.h>
#include <sqlpp23/mysql/detail/result_handle.h>
#include <sqlpp23/mysql/sqlpp_mysql.h>
#include <cstdlib>
#include <iostream>
#include <memory>

namespace sqlpp::mysql {
class char_result_t {
  std::unique_ptr<detail::result_handle> _handle;
  char_result_row_t _char_result_row;

 public:
  char_result_t() = default;
  char_result_t(std::unique_ptr<detail::result_handle>&& handle)
      : _handle{std::move(handle)} {
    if (_invalid())
      throw sqlpp::exception{
          "MySQL: Constructing char_result without valid handle"};

    if (_handle->debug)
      std::cerr << "MySQL debug: Constructing result, clause/using.handle at "
                << _handle.get() << std::endl;
  }

  char_result_t(const char_result_t&) = delete;
  char_result_t(char_result_t&& rhs) = default;
  char_result_t& operator=(const char_result_t&) = delete;
  char_result_t& operator=(char_result_t&&) = default;
  ~char_result_t() = default;

  bool operator==(const char_result_t& rhs) const {
    return _handle == rhs._handle;
  }

  size_t size() const {
    return _handle ? mysql_num_rows(_handle->mysql_res) : size_t{};
  }

  template <typename ResultRow>
  void next(ResultRow& result_row) {
    if (_invalid()) {
      sqlpp::detail::result_row_bridge{}.invalidate(result_row);
      return;
    }

    if (next_impl()) {
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

  bool _invalid() const { return !_handle or !*_handle; }

  void read_field(size_t index, bool& value) {
    value = (_char_result_row.data[index][0] == 't' or
             _char_result_row.data[index][0] == '1');
  }

  void read_field(size_t index, double& value) {
    value = std::strtod(_char_result_row.data[index], nullptr);
  }

  void read_field(size_t index, int64_t& value) {
    value = std::strtoll(_char_result_row.data[index], nullptr, 10);
  }

  void read_field(size_t index, uint64_t& value) {
    value = std::strtoull(_char_result_row.data[index], nullptr, 10);
  }

  void read_field(size_t index, std::span<const uint8_t>& value) {
    value = std::span<const uint8_t>(
        reinterpret_cast<const uint8_t*>(_char_result_row.data[index]),
        _char_result_row.len[index]);
  }

  void read_field(size_t index, std::string_view& value) {
    value = std::string_view(_char_result_row.data[index],
                             _char_result_row.len[index]);
  }

  void read_field(size_t index, ::sqlpp::chrono::day_point& value) {
    if (_handle->debug)
      std::cerr << "MySQL debug: parsing date result at index: " << index
                << std::endl;

    const auto date_string = _char_result_row.data[index];
    if (_handle->debug)
      std::cerr << "MySQL debug: date string: " << date_string << std::endl;

    if (::sqlpp::detail::parse_date(value, date_string) == false) {
      if (_handle->debug)
        std::cerr << "MySQL debug: invalid date result: " << date_string
                  << std::endl;
    }
  }

  void read_field(size_t index, ::sqlpp::chrono::microsecond_point& value) {
    if (_handle->debug)
      std::cerr << "MySQL debug: parsing date result at index: " << index
                << std::endl;

    const auto date_time_string = _char_result_row.data[index];
    if (_handle->debug)
      std::cerr << "MySQL debug: date_time string: " << date_time_string
                << std::endl;

    if (::sqlpp::detail::parse_timestamp(value, date_time_string) == false) {
      if (_handle->debug)
        std::cerr << "MySQL debug: invalid date_time result: "
                  << date_time_string << std::endl;
    }
  }

  void read_field(size_t index, ::std::chrono::microseconds& value) {
    if (_handle->debug)
      std::cerr << "MySQL debug: parsing time of day result at index: " << index
                << std::endl;

    const auto time_string = _char_result_row.data[index];
    if (_handle->debug)
      std::cerr << "MySQL debug: time of day string: " << time_string
                << std::endl;

    if (::sqlpp::detail::parse_time_of_day(value, time_string) == false) {
      if (_handle->debug)
        std::cerr << "MySQL debug: invalid time result: " << time_string
                  << std::endl;
    }
  }

  template <typename T>
  auto read_field(size_t index, std::optional<T>& value) -> void {
    const bool is_null = _char_result_row.data[index] == nullptr;
    if (is_null) {
      value.reset();
    } else {
      if (not value.has_value()) {
        value = T{};
      }
      read_field(index, *value);
    }
  }

 private:
  bool next_impl() {
    if (_handle->debug)
      std::cerr << "MySQL debug: Accessing next row of handle at "
                << _handle.get() << std::endl;

    _char_result_row.data =
        const_cast<const char**>(mysql_fetch_row(_handle->mysql_res));
    _char_result_row.len = mysql_fetch_lengths(_handle->mysql_res);

    return _char_result_row.data;
  }
};
}  // namespace sqlpp::mysql
