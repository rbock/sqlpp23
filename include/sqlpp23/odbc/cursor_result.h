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
#include <cstring>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

#include <sql.h>
#include <sqlext.h>

#include <sqlpp23/core/chrono.h>
#include <sqlpp23/core/query/result_row.h>
#include <sqlpp23/odbc/database/connection_config.h>
#include <sqlpp23/odbc/database/exception.h>
#include <sqlpp23/odbc/detail/diagnostics.h>

namespace sqlpp::odbc {
// Reads the rows of an executed statement through an ODBC cursor.
//
// If all selected columns can be bound to fixed-size buffers (numeric,
// date/time, and text/blob columns whose declared maximum size fits into
// connection_config::max_bound_column_size), the columns are bound with
// SQLBindCol and connection_config::row_array_size rows are fetched per
// driver round trip (a "block cursor" in ODBC terms).
//
// Otherwise rows are fetched one at a time and each value is streamed with
// SQLGetData into a buffer that grows as needed.
class cursor_result_t {
 public:
  enum class column_type : unsigned char {
    boolean,
    int64,
    uint64,
    floating_point,
    text,
    blob,
    date,
    time_of_day,
    timestamp,
  };

 private:
  struct column_t {
    column_type type;
    size_t buffer_size{0};  // bytes per row (only used in block mode)
    size_t value_size{0};   // bytes of the current value (streaming mode)
    bool fetched{false};    // current row's value was streamed already
    std::vector<char> buffer;
    std::vector<SQLLEN> indicators;
  };

  std::shared_ptr<void> _statement;  // SQLHSTMT
  const connection_config* _config{nullptr};
  std::vector<column_t> _columns;
  // Registered with the driver (SQL_ATTR_ROWS_FETCHED_PTR), heap-allocated so
  // that the address stays stable when the result is moved.
  std::unique_ptr<SQLULEN> _fetched_row_count;
  void* _result_row_address{nullptr};
  bool _fetch_prepared{false};
  bool _block_mode{false};
  size_t _rows_in_buffer{0};
  size_t _current_row{0};

 public:
  cursor_result_t() = default;
  cursor_result_t(std::shared_ptr<void> statement,
                  const connection_config* config)
      : _statement{std::move(statement)}, _config{config} {
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::result,
                         "ODBC: constructing cursor result, using handle at {}",
                         std::hash<void*>{}(_statement.get()));
    }
  }

  cursor_result_t(const cursor_result_t&) = delete;
  cursor_result_t(cursor_result_t&& rhs) = default;
  cursor_result_t& operator=(const cursor_result_t&) = delete;
  cursor_result_t& operator=(cursor_result_t&& rhs) {
    if (this != &rhs) {
      release_cursor();
      _statement = std::move(rhs._statement);
      _config = rhs._config;
      _columns = std::move(rhs._columns);
      _fetched_row_count = std::move(rhs._fetched_row_count);
      _result_row_address = rhs._result_row_address;
      _fetch_prepared = rhs._fetch_prepared;
      _block_mode = rhs._block_mode;
      _rows_in_buffer = rhs._rows_in_buffer;
      _current_row = rhs._current_row;
    }
    return *this;
  }

  ~cursor_result_t() { release_cursor(); }

  bool operator==(const cursor_result_t& rhs) const {
    return _statement == rhs._statement;
  }

  template <typename ResultRow>
  void next(ResultRow& result_row) {
    if (not _statement) {
      sqlpp::detail::result_row_bridge{}.invalidate(result_row);
      return;
    }

    if (&result_row != _result_row_address) {
      // Records the column plan via the bind_field functions below.
      sqlpp::detail::result_row_bridge{}.bind_fields(result_row, *this);
      if (not _fetch_prepared) {
        prepare_fetch();
        _fetch_prepared = true;
      }
      _result_row_address = &result_row;
    }

    if (next_row()) {
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

  const debug_logger& debug() const { return _config->debug; }

  void plan_column(size_t field_index, column_type type) {
    if (_columns.size() <= field_index) {
      _columns.resize(field_index + 1);
    }
    auto& column = _columns[field_index];
    column.type = type;
    column.buffer_size = fixed_buffer_size(type);  // 0 for text and blob
  }

  bool get_is_null(size_t field_index) {
    auto& column = _columns[field_index];
    if (_block_mode) {
      return column.indicators[_current_row] == SQL_NULL_DATA;
    }
    stream_value_if_required(field_index);
    return column.indicators[0] == SQL_NULL_DATA;
  }

  bool get_bool(size_t field_index) {
    return get_fixed<unsigned char>(field_index) != 0;
  }

  int64_t get_int64(size_t field_index) {
    return get_fixed<int64_t>(field_index);
  }

  uint64_t get_uint64(size_t field_index) {
    return get_fixed<uint64_t>(field_index);
  }

  double get_double(size_t field_index) {
    return get_fixed<double>(field_index);
  }

  SQL_DATE_STRUCT get_date(size_t field_index) {
    return get_fixed<SQL_DATE_STRUCT>(field_index);
  }

  SQL_TIME_STRUCT get_time(size_t field_index) {
    return get_fixed<SQL_TIME_STRUCT>(field_index);
  }

  SQL_TIMESTAMP_STRUCT get_timestamp(size_t field_index) {
    return get_fixed<SQL_TIMESTAMP_STRUCT>(field_index);
  }

  std::string_view get_text(size_t field_index) {
    const auto value = get_variable(field_index);
    return {value.data(), value.size()};
  }

  std::span<const uint8_t> get_blob(size_t field_index) {
    const auto value = get_variable(field_index);
    return {reinterpret_cast<const uint8_t*>(value.data()), value.size()};
  }

 private:
  SQLHSTMT native_handle() const { return _statement.get(); }

  static size_t fixed_buffer_size(column_type type) {
    switch (type) {
      case column_type::boolean:
        return 1;
      case column_type::int64:
      case column_type::uint64:
      case column_type::floating_point:
        return 8;
      case column_type::date:
        return sizeof(SQL_DATE_STRUCT);
      case column_type::time_of_day:
        return sizeof(SQL_TIME_STRUCT);
      case column_type::timestamp:
        return sizeof(SQL_TIMESTAMP_STRUCT);
      case column_type::text:
      case column_type::blob:
        return 0;
    }
    return 0;
  }

  static SQLSMALLINT c_type(column_type type) {
    switch (type) {
      case column_type::boolean:
        return SQL_C_BIT;
      case column_type::int64:
        return SQL_C_SBIGINT;
      case column_type::uint64:
        return SQL_C_UBIGINT;
      case column_type::floating_point:
        return SQL_C_DOUBLE;
      case column_type::date:
        return SQL_C_TYPE_DATE;
      case column_type::time_of_day:
        return SQL_C_TYPE_TIME;
      case column_type::timestamp:
        return SQL_C_TYPE_TIMESTAMP;
      case column_type::text:
        return SQL_C_CHAR;
      case column_type::blob:
        return SQL_C_BINARY;
    }
    return SQL_C_DEFAULT;
  }

  // Decides between block fetch and row-by-row streaming and sets up the
  // column buffers accordingly.
  void prepare_fetch() {
    bool all_columns_bindable = true;
    for (size_t index = 0; index < _columns.size(); ++index) {
      auto& column = _columns[index];
      if (column.buffer_size != 0) {
        continue;  // fixed-size column
      }
      SQLULEN declared_size{0};
      SQLSMALLINT sql_type{}, decimal_digits{}, nullable{}, name_length{};
      SQLCHAR name[256];
      detail::throw_on_error(
          SQLDescribeCol(native_handle(), static_cast<SQLUSMALLINT>(index + 1),
                         name, sizeof(name), &name_length, &sql_type,
                         &declared_size, &decimal_digits, &nullable),
          "ODBC: could not describe result column", SQL_HANDLE_STMT,
          native_handle());
      // Text sizes are reported in characters which can take up to 4 bytes
      // each (UTF-8), plus the terminating NUL.
      const size_t required_bytes = column.type == column_type::text
                                        ? declared_size * 4 + 1
                                        : declared_size;
      if (declared_size == 0 or
          required_bytes > _config->max_bound_column_size) {
        all_columns_bindable = false;
      } else {
        column.buffer_size = required_bytes;
      }
    }

    _block_mode = all_columns_bindable;
    if (not _block_mode) {
      if constexpr (debug_enabled) {
        _config->debug.log(log_category::result,
                           "ODBC: streaming rows one at a time (result "
                           "contains long variable-size columns)");
      }
      for (auto& column : _columns) {
        column.indicators.resize(1);
        // Text/blob buffers grow on demand while streaming, starting with
        // the declared size if it is known.
        column.buffer.resize(column.buffer_size);
      }
      return;
    }

    size_t rowset_size =
        _config->row_array_size > 0 ? _config->row_array_size : 1;
    if (rowset_size > 1) {
      const auto rc = SQLSetStmtAttr(
          native_handle(), SQL_ATTR_ROW_ARRAY_SIZE,
          reinterpret_cast<SQLPOINTER>(static_cast<SQLULEN>(rowset_size)), 0);
      if (rc != SQL_SUCCESS and rc != SQL_SUCCESS_WITH_INFO) {
        // The driver does not support block cursors.
        rowset_size = 1;
      } else {
        // The driver may have lowered the value (SQLSTATE 01S02).
        SQLULEN actual_size{1};
        if (SQLGetStmtAttr(native_handle(), SQL_ATTR_ROW_ARRAY_SIZE,
                           &actual_size, SQL_IS_UINTEGER,
                           nullptr) == SQL_SUCCESS) {
          rowset_size = actual_size;
        }
      }
    }
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::result,
                         "ODBC: fetching rows in blocks of {}", rowset_size);
    }

    _fetched_row_count = std::make_unique<SQLULEN>(0);
    detail::throw_on_error(
        SQLSetStmtAttr(native_handle(), SQL_ATTR_ROWS_FETCHED_PTR,
                       _fetched_row_count.get(), 0),
        "ODBC: could not register the fetched-row counter", SQL_HANDLE_STMT,
        native_handle());

    for (size_t index = 0; index < _columns.size(); ++index) {
      auto& column = _columns[index];
      column.buffer.resize(column.buffer_size * rowset_size);
      column.indicators.resize(rowset_size);
      detail::throw_on_error(
          SQLBindCol(native_handle(), static_cast<SQLUSMALLINT>(index + 1),
                     c_type(column.type), column.buffer.data(),
                     static_cast<SQLLEN>(column.buffer_size),
                     column.indicators.data()),
          "ODBC: could not bind result column", SQL_HANDLE_STMT,
          native_handle());
    }
  }

  bool next_row() {
    if (_current_row + 1 < _rows_in_buffer) {
      ++_current_row;
      return true;
    }

    if constexpr (debug_enabled) {
      _config->debug.log(log_category::result,
                         "ODBC: fetching next {} from the driver",
                         _block_mode ? "rowset" : "row");
    }
    const auto rc = SQLFetch(native_handle());
    if (rc == SQL_NO_DATA) {
      _rows_in_buffer = 0;
      _current_row = 0;
      return false;
    }
    detail::throw_on_error(rc, "ODBC: could not fetch rows", SQL_HANDLE_STMT,
                           native_handle());
    _rows_in_buffer = _block_mode ? *_fetched_row_count : 1;
    _current_row = 0;
    if (not _block_mode) {
      for (auto& column : _columns) {
        column.fetched = false;
      }
    }
    return _rows_in_buffer > 0;
  }

  template <typename T>
  T get_fixed(size_t field_index) {
    auto& column = _columns[field_index];
    T value{};
    if (_block_mode) {
      std::memcpy(&value,
                  column.buffer.data() + _current_row * column.buffer_size,
                  sizeof(T));
    } else {
      stream_value_if_required(field_index);
      std::memcpy(&value, column.buffer.data(), sizeof(T));
    }
    return value;
  }

  std::string_view get_variable(size_t field_index) {
    auto& column = _columns[field_index];
    if (_block_mode) {
      const auto indicator = column.indicators[_current_row];
      if (indicator == SQL_NULL_DATA) {
        return {};
      }
      // Text buffers reserve one byte for the terminating NUL.
      const size_t max_value_size = column.type == column_type::text
                                        ? column.buffer_size - 1
                                        : column.buffer_size;
      if (indicator == SQL_NO_TOTAL or
          static_cast<size_t>(indicator) > max_value_size) {
        throw exception{
            "ODBC: result value did not fit into the bound column buffer; "
            "increase connection_config::max_bound_column_size or fix the "
            "column's declared size",
            "01004"};
      }
      return {column.buffer.data() + _current_row * column.buffer_size,
              static_cast<size_t>(indicator)};
    }
    stream_value_if_required(field_index);
    return {column.buffer.data(), column.value_size};
  }

  // Streaming mode: retrieves the value of the given column of the current
  // row with SQLGetData. Columns are requested in increasing index order
  // (fields are read in select order), which every ODBC driver supports.
  void stream_value_if_required(size_t field_index) {
    auto& column = _columns[field_index];
    if (column.fetched) {
      return;
    }
    column.fetched = true;

    if (column.type != column_type::text and
        column.type != column_type::blob) {  // fixed-size column
      const auto rc = SQLGetData(
          native_handle(), static_cast<SQLUSMALLINT>(field_index + 1),
          c_type(column.type), column.buffer.data(),
          static_cast<SQLLEN>(column.buffer.size()), column.indicators.data());
      detail::throw_on_error(rc, "ODBC: could not get column data",
                             SQL_HANDLE_STMT, native_handle());
      return;
    }

    // Variable-size column: read in parts, growing the buffer as needed.
    const auto column_c_type = c_type(column.type);
    // SQLGetData NUL-terminates character data.
    const size_t terminator = column.type == column_type::text ? 1 : 0;
    if (column.buffer.size() < 256) {
      column.buffer.resize(256);
    }
    size_t received = 0;
    for (;;) {
      SQLLEN indicator{};
      const auto rc = SQLGetData(
          native_handle(), static_cast<SQLUSMALLINT>(field_index + 1),
          column_c_type, column.buffer.data() + received,
          static_cast<SQLLEN>(column.buffer.size() - received), &indicator);
      if (rc == SQL_NO_DATA) {
        break;  // the previous part already contained the complete value
      }
      detail::throw_on_error(rc, "ODBC: could not get column data",
                             SQL_HANDLE_STMT, native_handle());
      if (indicator == SQL_NULL_DATA) {
        column.indicators[0] = SQL_NULL_DATA;
        column.value_size = 0;
        return;
      }
      if (rc == SQL_SUCCESS) {
        // The final part; the indicator holds its size.
        received += static_cast<size_t>(indicator);
        break;
      }
      // SQL_SUCCESS_WITH_INFO: the value was truncated to the buffer. The
      // indicator holds the remaining size before this call (or SQL_NO_TOTAL
      // if the driver does not know it).
      const size_t part = column.buffer.size() - received - terminator;
      received += part;
      if (indicator == SQL_NO_TOTAL) {
        column.buffer.resize(column.buffer.size() * 2);
      } else {
        column.buffer.resize(
            received + (static_cast<size_t>(indicator) - part) + terminator);
      }
    }
    column.indicators[0] = static_cast<SQLLEN>(received);
    column.value_size = received;
  }

  // Leaves the statement ready for re-execution: closes the cursor and
  // removes all references to buffers owned by this result.
  void release_cursor() noexcept {
    if (not _statement) {
      return;
    }
    SQLFreeStmt(native_handle(), SQL_CLOSE);
    SQLFreeStmt(native_handle(), SQL_UNBIND);
    SQLSetStmtAttr(native_handle(), SQL_ATTR_ROWS_FETCHED_PTR, nullptr, 0);
    SQLSetStmtAttr(native_handle(), SQL_ATTR_ROW_ARRAY_SIZE,
                   reinterpret_cast<SQLPOINTER>(SQLULEN{1}), 0);
  }
};

inline void bind_field(cursor_result_t& result,
                       size_t field_index,
                       bool& /*value*/) {
  result.plan_column(field_index, cursor_result_t::column_type::boolean);
}

inline void bind_field(cursor_result_t& result,
                       size_t field_index,
                       int64_t& /*value*/) {
  result.plan_column(field_index, cursor_result_t::column_type::int64);
}

inline void bind_field(cursor_result_t& result,
                       size_t field_index,
                       uint64_t& /*value*/) {
  result.plan_column(field_index, cursor_result_t::column_type::uint64);
}

inline void bind_field(cursor_result_t& result,
                       size_t field_index,
                       double& /*value*/) {
  result.plan_column(field_index, cursor_result_t::column_type::floating_point);
}

inline void bind_field(cursor_result_t& result,
                       size_t field_index,
                       std::string_view& /*value*/) {
  result.plan_column(field_index, cursor_result_t::column_type::text);
}

inline void bind_field(cursor_result_t& result,
                       size_t field_index,
                       std::span<const uint8_t>& /*value*/) {
  result.plan_column(field_index, cursor_result_t::column_type::blob);
}

inline void bind_field(cursor_result_t& result,
                       size_t field_index,
                       std::chrono::sys_days& /*value*/) {
  result.plan_column(field_index, cursor_result_t::column_type::date);
}

inline void bind_field(cursor_result_t& result,
                       size_t field_index,
                       std::chrono::microseconds& /*value*/) {
  result.plan_column(field_index, cursor_result_t::column_type::time_of_day);
}

inline void bind_field(cursor_result_t& result,
                       size_t field_index,
                       ::sqlpp::chrono::sys_microseconds& /*value*/) {
  result.plan_column(field_index, cursor_result_t::column_type::timestamp);
}

inline void read_field(cursor_result_t& result,
                       size_t field_index,
                       bool& value) {
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result,
                       "ODBC: reading boolean result at index {}", field_index);
  }
  value = result.get_bool(field_index);
}

inline void read_field(cursor_result_t& result,
                       size_t field_index,
                       int64_t& value) {
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result,
                       "ODBC: reading integral result at index {}",
                       field_index);
  }
  value = result.get_int64(field_index);
}

inline void read_field(cursor_result_t& result,
                       size_t field_index,
                       uint64_t& value) {
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result,
                       "ODBC: reading unsigned integral result at index {}",
                       field_index);
  }
  value = result.get_uint64(field_index);
}

inline void read_field(cursor_result_t& result,
                       size_t field_index,
                       double& value) {
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result,
                       "ODBC: reading floating_point result at index {}",
                       field_index);
  }
  value = result.get_double(field_index);
}

inline void read_field(cursor_result_t& result,
                       size_t field_index,
                       std::string_view& value) {
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result,
                       "ODBC: reading text result at index {}", field_index);
  }
  value = result.get_text(field_index);
}

inline void read_field(cursor_result_t& result,
                       size_t field_index,
                       std::span<const uint8_t>& value) {
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result,
                       "ODBC: reading blob result at index {}", field_index);
  }
  value = result.get_blob(field_index);
}

inline void read_field(cursor_result_t& result,
                       size_t field_index,
                       std::chrono::sys_days& value) {
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result,
                       "ODBC: reading date result at index {}", field_index);
  }
  const auto date = result.get_date(field_index);
  value = std::chrono::year{date.year} / std::chrono::month{date.month} /
          std::chrono::day{date.day};
}

inline void read_field(cursor_result_t& result,
                       size_t field_index,
                       std::chrono::microseconds& value) {
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result,
                       "ODBC: reading time of day result at index {}",
                       field_index);
  }
  const auto time = result.get_time(field_index);
  value = std::chrono::hours{time.hour} + std::chrono::minutes{time.minute} +
          std::chrono::seconds{time.second};
}

inline void read_field(cursor_result_t& result,
                       size_t field_index,
                       ::sqlpp::chrono::sys_microseconds& value) {
  if constexpr (debug_enabled) {
    result.debug().log(log_category::result,
                       "ODBC: reading timestamp result at index {}",
                       field_index);
  }
  const auto ts = result.get_timestamp(field_index);
  value = std::chrono::sys_days{std::chrono::year{ts.year} /
                                std::chrono::month{ts.month} /
                                std::chrono::day{ts.day}} +
          std::chrono::hours{ts.hour} + std::chrono::minutes{ts.minute} +
          std::chrono::seconds{ts.second} +
          // The fraction is in billionths of a second.
          std::chrono::duration_cast<std::chrono::microseconds>(
              std::chrono::nanoseconds{ts.fraction});
}

}  // namespace sqlpp::odbc
