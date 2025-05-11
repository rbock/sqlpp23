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
#include <sqlpp23/core/query/result_row.h>
#include <sqlpp23/mysql/detail/prepared_statement_handle.h>
#include <sqlpp23/mysql/sqlpp_mysql.h>

namespace sqlpp::mysql {
class bind_result_t {
  std::shared_ptr<detail::prepared_statement_handle_t> _handle;
  void* _result_row_address{nullptr};
  bool _require_bind = true;

 public:
  bind_result_t() = default;
  bind_result_t(
      const std::shared_ptr<detail::prepared_statement_handle_t>& handle)
      : _handle{handle} {
    if constexpr (debug_enabled) {
      if (_handle) {
        _handle->debug().log(
            log_category::result,
            "MySQL debug: Constructing bind result, using handle at ",
            std::hash<void*>{}(_handle.get()));
      }
    }
  }
  bind_result_t(const bind_result_t&) = delete;
  bind_result_t(bind_result_t&& rhs) = default;
  bind_result_t& operator=(const bind_result_t&) = delete;
  bind_result_t& operator=(bind_result_t&&) = default;
  ~bind_result_t() {
    if (_handle)
      mysql_stmt_free_result(_handle->mysql_stmt);
  }

  bool operator==(const bind_result_t& rhs) const {
    return _handle == rhs._handle;
  }

  template <typename ResultRow>
  void next(ResultRow& result_row) {
    if (_invalid()) {
      sqlpp::detail::result_row_bridge{}.invalidate(result_row);
      return;
    }

    if (&result_row != _result_row_address) {
      // bind row data to mysql bind data
      sqlpp::detail::result_row_bridge{}.bind_fields(result_row, *this);
      _result_row_address = &result_row;
    }

    if (_require_bind) {
      bind_impl();  // binds mysql statement to data
      _require_bind = false;
    }

    if (next_impl()) {
      if (not result_row) {
        sqlpp::detail::result_row_bridge{}.validate(result_row);
      }
      // translates bind_data to row data where required
      sqlpp::detail::result_row_bridge{}.read_fields(result_row, *this);
    } else {
      if (result_row) {
        sqlpp::detail::result_row_bridge{}.invalidate(result_row);
      }
    }
  }

  bool _invalid() const { return !_handle or !*_handle; }

  void bind_field(size_t index, bool& /*value*/) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "MySQL debug: binding boolean result at index: {}",
                           index);
    }

    auto& buffer{_handle->result_buffers[index]};
    new (&buffer.bool_) bool{};

    MYSQL_BIND& param{_handle->result_params[index]};
    param.buffer_type = MYSQL_TYPE_TINY;
    param.buffer = &buffer.bool_;
    param.buffer_length = sizeof(buffer.bool_);
    param.length = &buffer.length;
    param.is_null = &buffer.is_null;
    param.is_unsigned = false;
    param.error = &buffer.error;
  }

  void bind_field(size_t index, int64_t& /*value*/) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "MySQL debug: binding integral result at index: {}",
                           index);
    }

    auto& buffer{_handle->result_buffers[index]};
    new (&buffer.int64_) int64_t{};

    MYSQL_BIND& param{_handle->result_params[index]};
    param.buffer_type = MYSQL_TYPE_LONGLONG;
    param.buffer = &buffer.int64_;
    param.buffer_length = sizeof(buffer.int64_);
    param.length = &buffer.length;
    param.is_null = &buffer.is_null;
    param.is_unsigned = false;
    param.error = &buffer.error;
  }

  void bind_field(size_t index, uint64_t& /*value*/) {
    if constexpr (debug_enabled) {
      _handle->debug().log(
          log_category::result,
          "MySQL debug: binding unsigned integral result at index: {}", index);
    }

    auto& buffer{_handle->result_buffers[index]};
    new (&buffer.uint64_) uint64_t{};

    MYSQL_BIND& param{_handle->result_params[index]};
    param.buffer_type = MYSQL_TYPE_LONGLONG;
    param.buffer = &buffer.uint64_;
    param.buffer_length = sizeof(buffer.uint64_);
    param.length = &buffer.length;
    param.is_null = &buffer.is_null;
    param.is_unsigned = true;
    param.error = &buffer.error;
  }

  void bind_field(size_t index, double& /*value*/) {
    if constexpr (debug_enabled) {
      _handle->debug().log(
          log_category::result,
          "MySQL debug: binding floating point result at index: {}", index);
    }

    auto& buffer{_handle->result_buffers[index]};
    new (&buffer.double_) double{};

    MYSQL_BIND& param{_handle->result_params[index]};
    param.buffer_type = MYSQL_TYPE_DOUBLE;
    param.buffer = &buffer.double_;
    param.buffer_length = sizeof(buffer.double_);
    param.length = &buffer.length;
    param.is_null = &buffer.is_null;
    param.is_unsigned = false;
    param.error = &buffer.error;
  }

  void bind_field(size_t index, std::string_view& /*value*/) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "MySQL debug: binding text result at index: {}",
                           index);
    }

    auto& buffer{_handle->result_buffers[index]};

    MYSQL_BIND& param{_handle->result_params[index]};
    param.buffer_type = MYSQL_TYPE_STRING;
    param.buffer = buffer.var_buffer.data();
    param.buffer_length = buffer.var_buffer.size();
    param.length = &buffer.length;
    param.is_null = &buffer.is_null;
    param.is_unsigned = false;
    param.error = &buffer.error;
  }

  void bind_field(size_t index, std::span<const uint8_t>& /*value*/) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "MySQL debug: binding blob result at index: {}",
                           index);
    }

    auto& buffer{_handle->result_buffers[index]};

    MYSQL_BIND& param{_handle->result_params[index]};
    param.buffer_type = MYSQL_TYPE_BLOB;
    param.buffer = buffer.var_buffer.data();
    param.buffer_length = buffer.var_buffer.size();
    param.length = &buffer.length;
    param.is_null = &buffer.is_null;
    param.is_unsigned = false;
    param.error = &buffer.error;
  }

  void bind_chrono_field(size_t index, enum_field_types buffer_type) {
    auto& buffer{_handle->result_buffers[index]};
    new (&buffer.mysql_time_) MYSQL_TIME{};

    MYSQL_BIND& param{_handle->result_params[index]};
    param.buffer_type = buffer_type;
    param.buffer = &buffer.mysql_time_;
    param.buffer_length = sizeof(buffer.mysql_time_);
    param.length = &buffer.length;
    param.is_null = &buffer.is_null;
    param.is_unsigned = false;
    param.error = &buffer.error;
  }

  void bind_field(size_t index, ::sqlpp::chrono::day_point& /*value*/) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "MySQL debug: binding date result at index: {}",
                           index);
    }

    bind_chrono_field(index, MYSQL_TYPE_DATE);
  }

  void bind_field(size_t index, ::sqlpp::chrono::microsecond_point& /*value*/) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "MySQL debug: binding date time result at index: {}",
                           index);
    }

    bind_chrono_field(index, MYSQL_TYPE_DATETIME);
  }

  void bind_field(size_t index, ::std::chrono::microseconds& /*value*/) {
    if constexpr (debug_enabled) {
      _handle->debug().log(
          log_category::result,
          "MySQL debug: binding time of day result at index: {}", index);
    }

    bind_chrono_field(index, MYSQL_TYPE_TIME);
  }

  template <class T>
  void bind_field(size_t index, std::optional<T>& value) {
    value = T{};
    bind_field(index, *value);
  }

  void read_field(size_t index, bool& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "MySQL debug: reading bool result at index: {}",
                           index);
    }
    value = _handle->result_buffers[index].bool_;
  }

  void read_field(size_t index, int64_t& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "MySQL debug: reading integral result at index: {}",
                           index);
    }
    value = _handle->result_buffers[index].int64_;
  }

  void read_field(size_t index, uint64_t& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(
          log_category::result,
          "MySQL debug: reading unsigned integral result at index: {}", index);
    }
    value = _handle->result_buffers[index].uint64_;
  }

  void read_field(size_t index, double& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(
          log_category::result,
          "MySQL debug: reading floating point result at index: {}", index);
    }
    value = _handle->result_buffers[index].double_;
  }

  void refetch_if_required(size_t index) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "MySQL debug: Checking result size at index: {}",
                           index);
    }
    auto& buffer = _handle->result_buffers[index];
    auto& params = _handle->result_params[index];
    if (*params.length > params.buffer_length) {
      if constexpr (debug_enabled) {
        _handle->debug().log(log_category::result,
                             "MySQL debug: increasing buffer at: {} to {}",
                             index, *params.length);
      }

      buffer.var_buffer.resize(*params.length);
      params.buffer = buffer.var_buffer.data();
      params.buffer_length = buffer.var_buffer.size();
      const auto err =
          mysql_stmt_fetch_column(_handle->mysql_stmt, &params, index, 0);
      if (err)
        throw sqlpp::exception{
            std::string{"MySQL: Fetch column after reallocate failed: "} +
            "error-code: " + std::to_string(err) + ", stmt-error: " +
            mysql_stmt_error(_handle->mysql_stmt) + ", stmt-errno: " +
            std::to_string(mysql_stmt_errno(_handle->mysql_stmt))};
      _require_bind = true;
    }
  }

  void read_field(size_t index, std::string_view& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "MySQL debug: reading text result at index: {}",
                           index);
    }
    refetch_if_required(index);
    const auto& buffer = _handle->result_buffers[index];
    const auto& params = _handle->result_params[index];
    value = std::string_view(buffer.var_buffer.data(), *params.length);
  }

  void read_field(size_t index, std::span<const uint8_t>& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "MySQL debug: reading blob result at index: {}",
                           index);
    }
    refetch_if_required(index);
    const auto& buffer = _handle->result_buffers[index];
    const auto& params = _handle->result_params[index];
    value = std::span<const uint8_t>(
        reinterpret_cast<const uint8_t*>(buffer.var_buffer.data()),
        *params.length);
  }

  void read_field(size_t index, ::sqlpp::chrono::day_point& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "MySQL debug: reading date result at index: {}",
                           index);
    }

    const auto& dt = _handle->result_buffers[index].mysql_time_;
    if (dt.year > std::numeric_limits<int>::max())
      throw sqlpp::exception{"cannot read year from db: " +
                             std::to_string(dt.year)};
    value = ::std::chrono::year(static_cast<int>(dt.year)) /
            ::std::chrono::month(dt.month) / ::std::chrono::day(dt.day);
  }

  void read_field(size_t index, ::sqlpp::chrono::microsecond_point& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "MySQL debug: reading date time result at index: {}",
                           index);
    }

    const auto& dt = _handle->result_buffers[index].mysql_time_;
    if (dt.year > std::numeric_limits<int>::max())
      throw sqlpp::exception{"cannot read year from db: " +
                             std::to_string(dt.year)};
    value = ::sqlpp::chrono::day_point(
                ::std::chrono::year(static_cast<int>(dt.year)) /
                ::std::chrono::month(dt.month) / ::std::chrono::day(dt.day)) +
            std::chrono::hours(dt.hour) + std::chrono::minutes(dt.minute) +
            std::chrono::seconds(dt.second) +
            std::chrono::microseconds(dt.second_part);
  }

  void read_field(size_t index, ::std::chrono::microseconds& value) {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "MySQL debug: reading date time result at index: {}",
                           index);
    }

    const auto& dt = _handle->result_buffers[index].mysql_time_;
    value = std::chrono::hours(dt.hour) + std::chrono::minutes(dt.minute) +
            std::chrono::seconds(dt.second) +
            std::chrono::microseconds(dt.second_part);
  }

  template <class T>
  void read_field(size_t index, std::optional<T>& value) {
    if (_handle->result_buffers[index].is_null) {
      value.reset();
      return;
    }
    if (!value.has_value())
      value = T{};
    read_field(index, *value);
  }

 private:
  void bind_impl() {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "MySQL debug: Binding results for handle at {}",
                           std::hash<void*>{}(_handle.get()));
    }

    if (mysql_stmt_bind_result(_handle->mysql_stmt,
                               _handle->result_params.data())) {
      throw sqlpp::exception{std::string{"MySQL: mysql_stmt_bind_result: "} +
                             mysql_stmt_error(_handle->mysql_stmt)};
    }
  }

  bool next_impl() {
    if constexpr (debug_enabled) {
      _handle->debug().log(log_category::result,
                           "MySQL debug: Accessing next row of handle at {}",
                           std::hash<void*>{}(_handle.get()));
    }

    const auto flag = mysql_stmt_fetch(_handle->mysql_stmt);

    switch (flag) {
      case 0:
      case MYSQL_DATA_TRUNCATED:
        return true;
      case 1:
        throw sqlpp::exception{
            std::string{"MySQL: Could not fetch next result: "} +
            mysql_stmt_error(_handle->mysql_stmt)};
      case MYSQL_NO_DATA:
        return false;
      default:
        throw sqlpp::exception{
            "MySQL: Unexpected return value for mysql_stmt_fetch()"};
    }
  }
};
}  // namespace sqlpp::mysql
