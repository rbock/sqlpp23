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

#include <array>
#include <chrono>
#include <memory>
#include <span>
#include <string>
#include <string_view>

#include <sqlpp23/core/chrono.h>
#include <sqlpp23/core/detail/parse_date_time.h>
#include <sqlpp23/core/query/result_row.h>
#include <sqlpp23/duck_db/database/connection_config.h>
#include <sqlpp23/duck_db/database/exception.h>

namespace sqlpp::duck_db {

namespace detail {
// Keeps a duckdb_result alive and remembers how many rows it holds. DuckDB's C
// API materializes non-streaming results, so a simple row counter can be used
// for iteration.
//
// IMPORTANT: the duckdb_result must be owned by this holder and must never be
// copied or moved. Copying/moving the struct copies the pointer to the internal
// result data without transferring ownership of the materialized columns, which
// silently turns every value access into a default value.
struct result_holder {
  ::duckdb_result result{};
  uint64_t row_count = 0;
  bool is_streaming = false;

  result_holder() = default;
  explicit result_holder(::duckdb_result* r) : result{*r} {
    is_streaming = ::duckdb_result_is_streaming(result);
    row_count = is_streaming ? 0 : ::duckdb_row_count(&result);
  }
  result_holder(const result_holder&) = delete;
  result_holder& operator=(const result_holder&) = delete;
  ~result_holder() { ::duckdb_destroy_result(&result); }
};

using result_holder_ptr_t = std::shared_ptr<result_holder>;
}  // namespace detail

class bind_result_t {
 public:
  using result_holder_ptr_t = detail::result_holder_ptr_t;

 private:
  ::duckdb_connection _connection = nullptr;
  detail::result_holder_ptr_t _result;
  const connection_config* _config = nullptr;
  uint64_t _row_index = 0;

 public:
  bind_result_t() = default;
  bind_result_t(::duckdb_connection connection,
                detail::result_holder_ptr_t result,
                const connection_config* config)
      : _connection{connection}, _result{std::move(result)}, _config{config} {
    if constexpr (debug_enabled) {
      if (_config and _result and _result->is_streaming) {
        _config->debug.log(log_category::result,
                           "DuckDB: the result is streaming, row-by-row access "
                           "is not supported by this connector");
      }
    }
  }

  bind_result_t(const bind_result_t&) = delete;
  bind_result_t(bind_result_t&& rhs) = default;
  bind_result_t& operator=(const bind_result_t&) = delete;
  bind_result_t& operator=(bind_result_t&& rhs) = default;
  ~bind_result_t() = default;

  bool operator==(const bind_result_t& rhs) const {
    return _result == rhs._result;
  }

  template <typename ResultRow>
  void next(ResultRow& result_row) {
    if (not _result or _row_index >= _result->row_count) {
      if constexpr (debug_enabled) {
        _config->debug.log(log_category::result,
                           "DuckDB debug: no more rows (row {} of {})",
                           _row_index, _result ? _result->row_count : 0);
      }
      if (result_row) {
        sqlpp::detail::result_row_bridge{}.invalidate(result_row);
      }
      return;
    }

    // _row_index is the row that the accessors read from. It is advanced only
    // after the fields of the current row have been read.
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::result,
                         "DuckDB debug: accessing row {} of {}", _row_index,
                         _result->row_count);
    }
    if (not result_row) {
      sqlpp::detail::result_row_bridge{}.validate(result_row);
    }
    sqlpp::detail::result_row_bridge{}.read_fields(result_row, *this);
    ++_row_index;
  }

  auto& debug() const { return _config->debug; }

  uint64_t get_row() const { return _row_index; }
  uint64_t get_row_count() const { return _result ? _result->row_count : 0; }
  size_t get_column_count() const {
    return _result ? ::duckdb_column_count(&_result->result) : 0;
  }
  const char* get_column_name(size_t column_index) const {
    return _result ? ::duckdb_column_name(&_result->result,
                                         static_cast<::idx_t>(column_index))
                   : nullptr;
  }
  ::duckdb_type get_type(size_t column_index) const {
    return _result ? ::duckdb_column_type(&_result->result,
                                          static_cast<::idx_t>(column_index))
                   : DUCKDB_TYPE_INVALID;
  }

  bool get_is_null(size_t column_index) const {
    return ::duckdb_value_is_null(&_result->result,
                                  static_cast<::idx_t>(column_index),
                                  static_cast<::idx_t>(_row_index));
  }
  bool get_bool(size_t column_index) const {
    return ::duckdb_value_boolean(&_result->result,
                                  static_cast<::idx_t>(column_index),
                                  static_cast<::idx_t>(_row_index));
  }
  int64_t get_int64(size_t column_index) const {
    return ::duckdb_value_int64(&_result->result,
                                static_cast<::idx_t>(column_index),
                                static_cast<::idx_t>(_row_index));
  }
  uint64_t get_uint64(size_t column_index) const {
    return ::duckdb_value_uint64(&_result->result,
                                 static_cast<::idx_t>(column_index),
                                 static_cast<::idx_t>(_row_index));
  }
  double get_double(size_t column_index) const {
    return ::duckdb_value_double(&_result->result,
                                 static_cast<::idx_t>(column_index),
                                 static_cast<::idx_t>(_row_index));
  }
  // Returns a pointer into the result's own memory, valid as long as the
  // result is alive.
  ::duckdb_string get_varchar(size_t column_index) const {
    return ::duckdb_value_string_internal(&_result->result,
                                          static_cast<::idx_t>(column_index),
                                          static_cast<::idx_t>(_row_index));
  }
  ::duckdb_blob get_blob(size_t column_index) const {
    return ::duckdb_value_blob(&_result->result,
                               static_cast<::idx_t>(column_index),
                               static_cast<::idx_t>(_row_index));
  }
  ::duckdb_date get_date(size_t column_index) const {
    return ::duckdb_value_date(&_result->result,
                               static_cast<::idx_t>(column_index),
                               static_cast<::idx_t>(_row_index));
  }
  ::duckdb_time get_time(size_t column_index) const {
    return ::duckdb_value_time(&_result->result,
                               static_cast<::idx_t>(column_index),
                               static_cast<::idx_t>(_row_index));
  }
  ::duckdb_timestamp get_timestamp(size_t column_index) const {
    return ::duckdb_value_timestamp(&_result->result,
                                    static_cast<::idx_t>(column_index),
                                    static_cast<::idx_t>(_row_index));
  }
};

inline void read_field(const bind_result_t& result, size_t index, bool& value) {
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result,
                       "DuckDB debug: binding boolean result at index {}",
                       index);
  }

  value = result.get_bool(index);
}

inline void read_field(const bind_result_t& result,
                       size_t index,
                       double& value) {
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result,
                       "DuckDB debug: binding floating_point result at index "
                       "{}",
                       index);
  }

  value = result.get_double(index);
}

inline void read_field(const bind_result_t& result,
                       size_t index,
                       int64_t& value) {
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result,
                       "DuckDB debug: reading integral result at index {}",
                       index);
  }

  value = result.get_int64(index);
}

inline void read_field(const bind_result_t& result,
                       size_t index,
                       uint64_t& value) {
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result,
                       "DuckDB debug: binding unsigned integral result at "
                       "index {}",
                       index);
  }

  value = result.get_uint64(index);
}

inline void read_field(const bind_result_t& result,
                       size_t index,
                       std::string_view& value) {
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result,
                       "DuckDB debug: binding text result at index {}", index);
  }

  const auto text = result.get_varchar(index);
  value = std::string_view(text.data ? text.data : "",
                           static_cast<size_t>(text.size));
}

inline void read_field(const bind_result_t& result,
                       size_t index,
                       std::span<const uint8_t>& value) {
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result,
                       "DuckDB debug: binding blob result at index {}", index);
  }

  const auto blob = result.get_blob(index);
  value = std::span<const uint8_t>(static_cast<const uint8_t*>(blob.data),
                                   static_cast<size_t>(blob.size));
}

inline void read_field(const bind_result_t& result,
                       size_t index,
                       std::chrono::microseconds& value) {
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result,
                       "DuckDB debug: binding time result at index {}", index);
  }

  if (result.get_type(index) == DUCKDB_TYPE_TIME) {
    value = std::chrono::microseconds{result.get_time(index).micros};
    return;
  }

  // Fall back to parsing text representations for other column types.
  std::string_view text;
  read_field(result, index, text);
  const char* time_string = text.data();
  if (::sqlpp::detail::parse_time(value, time_string) == false) {
    value = {};
    if constexpr (debug_enabled) {
      result.debug().log(log_category::result, "DuckDB debug: invalid time");
    }
  }
}

inline void read_field(const bind_result_t& result,
                       size_t index,
                       std::chrono::sys_days& value) {
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result,
                       "DuckDB debug: binding date result at index {}", index);
  }

  if (result.get_type(index) == DUCKDB_TYPE_DATE) {
    // DuckDB stores dates as days since 1970-01-01, exactly like sys_days.
    value = std::chrono::sys_days{std::chrono::days{result.get_date(index).days}};
    return;
  }

  std::string_view text;
  read_field(result, index, text);
  const char* date_string = text.data();
  if (::sqlpp::detail::parse_date(value, date_string) == false) {
    value = {};
    if constexpr (debug_enabled) {
      result.debug().log(log_category::result, "DuckDB debug: invalid date");
    }
  }
}

inline void read_field(const bind_result_t& result,
                       size_t index,
                       ::sqlpp::chrono::sys_microseconds& value) {
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result,
                       "DuckDB debug: binding date_time result at index {}",
                       index);
  }

  if (result.get_type(index) == DUCKDB_TYPE_TIMESTAMP) {
    value = ::sqlpp::chrono::sys_microseconds{
        std::chrono::microseconds{result.get_timestamp(index).micros}};
    return;
  }

  std::string_view text;
  read_field(result, index, text);
  const char* date_time_string = text.data();
  if (::sqlpp::detail::parse_timestamp(value, date_time_string) == false) {
    value = {};
    if constexpr (debug_enabled) {
      result.debug().log(log_category::result,
                         "DuckDB debug: invalid date_time");
    }
  }
}

}  // namespace sqlpp::duck_db
