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

#include <memory>
#include <optional>
#include <span>
#include <string_view>

#include <sqlpp23/core/chrono.h>
#include <sqlpp23/core/database/exception.h>
#include <sqlpp23/core/detail/parse_date_time.h>
#include <sqlpp23/core/query/result_row.h>
#include <sqlpp23/sqlite3/detail/prepared_statement_handle.h>
#include <sqlpp23/sqlite3/export.h>

#ifdef _MSC_VER
#include <iso646.h>
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace sqlpp::sqlite3 {
class SQLPP11_SQLITE3_EXPORT bind_result_t {
  std::shared_ptr<detail::prepared_statement_handle_t> _handle;

 public:
  bind_result_t() = default;
  bind_result_t(
      const std::shared_ptr<detail::prepared_statement_handle_t>& handle)
      : _handle{handle} {
    if constexpr (debug_enabled) {
      if (_handle) {
        _handle->debug().log(
            log_category::result,
            "Sqlite3 debug: Constructing bind result, using handle at {}",
            std::hash<void*>{}(_handle.get()));
      }
    }
  }

  bind_result_t(const bind_result_t&) = delete;
  bind_result_t(bind_result_t&& rhs) = default;
  bind_result_t& operator=(const bind_result_t&) = delete;
  bind_result_t& operator=(bind_result_t&&) = default;
  ~bind_result_t() = default;

  bool operator==(const bind_result_t& rhs) const {
    return _handle == rhs._handle;
  }

  template <typename ResultRow>
  void next(ResultRow& result_row) {
    if (!_handle) {
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

  void read_field(size_t index, bool& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "Sqlite3 debug: binding boolean result at index {}",
                           index);
    }

    value = static_cast<signed char>(
        sqlite3_column_int(_handle->sqlite_statement, static_cast<int>(index)));
  }

  void read_field(size_t index, double& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(
          log_category::result,
          "Sqlite3 debug: binding floating_point result at index {}", index);
    }

    switch (sqlite3_column_type(_handle->sqlite_statement,
                                static_cast<int>(index))) {
      case (SQLITE3_TEXT):
        value = atof(reinterpret_cast<const char*>(sqlite3_column_text(
            _handle->sqlite_statement, static_cast<int>(index))));
        break;
      default:
        value = sqlite3_column_double(_handle->sqlite_statement,
                                      static_cast<int>(index));
    }
  }

  void read_field(size_t index, int64_t& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "Sqlite3 debug: reading integral result at index {}",
                           index);
    }

    value = sqlite3_column_int64(_handle->sqlite_statement,
                                 static_cast<int>(index));
  }

  void read_field(size_t index, uint64_t& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(
          log_category::result,
          "Sqlite3 debug: binding unsigned integral result at index {}", index);
    }

    value = static_cast<uint64_t>(sqlite3_column_int64(
        _handle->sqlite_statement, static_cast<int>(index)));
  }

  void read_field(size_t index, std::string_view& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "Sqlite3 debug: binding text result at index {}",
                           index);
    }

    value = std::string_view(
        reinterpret_cast<const char*>(sqlite3_column_text(
            _handle->sqlite_statement, static_cast<int>(index))),
        static_cast<size_t>(sqlite3_column_bytes(_handle->sqlite_statement,
                                                 static_cast<int>(index))));
  }

  void read_field(size_t index, std::span<const uint8_t>& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "Sqlite3 debug: binding blob result at index {}",
                           index);
    }

    value = std::span<const uint8_t>(
        reinterpret_cast<const uint8_t*>(sqlite3_column_blob(
            _handle->sqlite_statement, static_cast<int>(index))),
        static_cast<size_t>(sqlite3_column_bytes(_handle->sqlite_statement,
                                                 static_cast<int>(index))));
  }

  void read_field(size_t index, std::chrono::microseconds& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "Sqlite3 debug: binding date result at index {}",
                           index);
    }

    const auto time_of_day_string =
        reinterpret_cast<const char*>(sqlite3_column_text(
            _handle->sqlite_statement, static_cast<int>(index)));
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "Sqlite3 debug: time_of_day string {}",
                           time_of_day_string);
    }

    if (::sqlpp::detail::parse_time_of_day(value, time_of_day_string) ==
        false) {
      value = {};
      if constexpr (debug_enabled) {
        _handle->debug().log(log_category::result,
                             "Sqlite3 debug: invalid date result {}",
                             time_of_day_string);
      }
    }
  }

  void read_field(size_t index, ::sqlpp::chrono::day_point& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "Sqlite3 debug: binding date result at index {}",
                           index);
    }

    const auto date_string = reinterpret_cast<const char*>(sqlite3_column_text(
        _handle->sqlite_statement, static_cast<int>(index)));
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "Sqlite3 debug: date string: {}", date_string);
    }

    if (::sqlpp::detail::parse_date(value, date_string) == false) {
      value = {};
      if constexpr (debug_enabled) {
        _handle->debug().log(log_category::result,
                             "Sqlite3 debug: invalid date result: {}",
                             date_string);
      }
    }
  }

  void read_field(size_t index, ::sqlpp::chrono::microsecond_point& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "Sqlite3 debug: binding date result at index {}",
                           index);
    }

    const auto date_time_string =
        reinterpret_cast<const char*>(sqlite3_column_text(
            _handle->sqlite_statement, static_cast<int>(index)));
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "Sqlite3 debug: date_time string: {}",
                           date_time_string);
    }

    // We treat DATETIME fields as containing either date+time or just date.
    if (::sqlpp::detail::parse_date_or_timestamp(value, date_time_string) ==
        false) {
      value = {};
      if constexpr (debug_enabled) {
        _handle->debug().log(log_category::result,
                             "Sqlite3 debug: invalid date_time result: {}",
                             date_time_string);
      }
    }
  }

  template <typename T>
  auto read_field(size_t index, std::optional<T>& field) -> void {
    const bool is_null =
        sqlite3_column_type(_handle->sqlite_statement,
                            static_cast<int>(index)) == SQLITE_NULL;
    if (is_null) {
      field.reset();
    } else {
      if (not field.has_value()) {
        field = T{};
      }
      read_field(index, *field);
    }
  }

 private:
  bool next_impl() {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "Sqlite3 debug: Accessing next row of handle at ",
                           std::hash<void*>{}(_handle.get()));
    }

    auto rc = sqlite3_step(_handle->sqlite_statement);

    switch (rc) {
      case SQLITE_ROW:
        return true;
      case SQLITE_DONE:
        return false;
      default:
        throw sqlpp::exception{
            "Sqlite3 error: Unexpected return value for sqlite3_step()"};
    }
  }
};
}  // namespace sqlpp::sqlite3

#ifdef _MSC_VER
#pragma warning(pop)
#endif
