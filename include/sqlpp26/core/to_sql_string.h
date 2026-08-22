#pragma once

/*
 * Copyright (c) 2024, Roland Bock
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

#include <array>
#include <cmath>
#include <format>
#include <optional>
#include <span>
#include <string>
#include <string_view>

#include <sqlpp26/core/chrono.h>
#include <sqlpp26/core/database/exception.h>
#include <sqlpp26/core/type_traits.h>
#include <sqlpp26/core/wrong.h>

namespace sqlpp {
template <typename Context, typename X = void>
auto to_sql_string(Context&, ...) -> std::string {
  static_assert(wrong<X>, "Missing specialization");
  return {};
}

template <typename Context>
auto to_sql_string(Context&, const bool& t) -> std::string {
  return std::to_string(t);
}

template <typename Context>
auto to_sql_string(Context&, const int8_t& t) -> std::string {
  return std::to_string(t);
}

template <typename Context>
auto to_sql_string(Context&, const int16_t& t) -> std::string {
  return std::to_string(t);
}

template <typename Context>
auto to_sql_string(Context&, const int32_t& t) -> std::string {
  return std::to_string(t);
}

template <typename Context>
auto to_sql_string(Context&, const int64_t& t) -> std::string {
  return std::to_string(t);
}

template <typename Context>
auto to_sql_string(Context&, const uint8_t& t) -> std::string {
  return std::to_string(t);
}

template <typename Context>
auto to_sql_string(Context&, const uint16_t& t) -> std::string {
  return std::to_string(t);
}

template <typename Context>
auto to_sql_string(Context&, const uint32_t& t) -> std::string {
  return std::to_string(t);
}

template <typename Context>
auto to_sql_string(Context&, const uint64_t& t) -> std::string {
  return std::to_string(t);
}

template <typename Context>
auto nan_to_sql_string(Context&) -> std::string {
  throw ::sqlpp::exception(
      "Serialization of NaN is not supported by this connector");
}

template <typename Context>
auto inf_to_sql_string(Context&) -> std::string {
  throw ::sqlpp::exception(
      "Serialization of Infinity is not supported by this connector");
}

template <typename Context>
auto neg_inf_to_sql_string(Context&) -> std::string {
  throw ::sqlpp::exception(
      "Serialization of Infinity is not supported by this connector");
}

template <typename Context, typename T>
auto float_to_sql_string(Context& context, const T& f) -> std::string {
  if (std::isnan(f)) {
    return nan_to_sql_string(context);
  } else if (std::isinf(f)) {
    return f > std::numeric_limits<T>::max() ? inf_to_sql_string(context)
                                             : neg_inf_to_sql_string(context);
  } else {
    return std::format("{:.{}g}", f, std::numeric_limits<T>::digits10);
  }
}

template <typename Context>
auto to_sql_string(Context& context, const float& t) -> std::string {
  return float_to_sql_string(context, t);
}

template <typename Context>
auto to_sql_string(Context& context, const double& t) -> std::string {
  return float_to_sql_string(context, t);
}

template <typename Context>
auto to_sql_string(Context& context, const long double& t) -> std::string {
  return float_to_sql_string(context, t);
}

template <typename Context>
auto to_sql_string(Context& context, const std::string_view& t) -> std::string {
  return "'" + context.escape(t) + "'";
}

template <typename Context>
auto to_sql_string(Context& context, const char* t) -> std::string {
  return to_sql_string(context, std::string_view(t));
}

template <typename Context>
auto to_sql_string(Context& context, const std::string& t) -> std::string {
  return to_sql_string(context, std::string_view(t));
}

template <typename Context>
auto to_sql_string(Context& context, const char& t) -> std::string {
  return to_sql_string(context, std::string_view(&t, 1));
}

// MySQL and sqlite3 use x'...', but PostgreSQL uses '\x...' to encode
// hexadecimal literals See
//  - https://dev.mysql.com/doc/refman/9.0/en/hexadecimal-literals.html
//  - https://www.sqlite.org/lang_expr.html
//  -
//  https://www.postgresql.org/docs/current/datatype-binary.html#DATATYPE-BINARY-BYTEA-HEX-FORMAT
//
// The PostgreSQL connector therefore specializes this function.
template <typename Context>
auto to_sql_string(Context&, const std::span<const uint8_t>& t) -> std::string {
  constexpr char hexChars[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                 '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  auto result = std::string{"x'"};
  result.reserve(t.size() * 2 + 3);

  for (const auto c : t) {
    result.push_back(hexChars[c >> 4]);
    result.push_back(hexChars[c & 0x0F]);
  }
  result.push_back('\'');

  return result;
}

template <typename Context>
auto to_sql_string(Context& context, const std::vector<uint8_t>& t)
    -> std::string {
  return to_sql_string(context, std::span<const uint8_t>(begin(t), end(t)));
}

template <typename Context, std::size_t N>
auto to_sql_string(Context& context, const std::array<uint8_t, N>& t)
    -> std::string {
  return to_sql_string(context, std::span<const uint8_t>(t.data(), t.size()));
}

template <typename Context>
auto to_sql_string(Context&, const std::chrono::sys_days& t)
    -> std::string {
  return std::format("DATE '{0:%Y-%m-%d}'", t);
}

template <typename Context>
auto to_sql_string(Context&, const std::chrono::microseconds& t)
    -> std::string {
  return std::format("'{0:%H:%M:%S}'", t);
}

template <typename Period, typename Context>
auto to_sql_string(
    Context&,
    const std::chrono::time_point<std::chrono::system_clock, Period>& t)
    -> std::string {
  return std::format("TIMESTAMP '{0:%Y-%m-%dT%H:%M:%S}'", t);
}

template <typename Context>
auto to_sql_string(Context&, const std::nullopt_t&) -> std::string {
  return "NULL";
}

template <typename T, typename Context>
auto to_sql_string(Context& context, const std::optional<T>& t) -> std::string {
  if (not t.has_value()) {
    return to_sql_string(context, std::nullopt);
  }
  return to_sql_string(context, *t);
}

template <typename T, typename Context>
auto operand_to_sql_string(Context& context, const T& t) -> std::string {
  if (requires_parentheses<T>::value) {
    return "(" + to_sql_string(context, t) + ")";
  }
  return to_sql_string(context, t);
}

template <typename Context>
auto quoted_name_to_sql_string(Context&, const std::string_view& name)
    -> std::string {
  return '"' + std::string(name) + '"';
}

template <typename Context, std::size_t N>
auto name_to_sql_string(Context& /*context*/, const fixed_string<N>& name) -> std::string {
  // TODO: if (NameTag::require_quotes) {
    //return quoted_name_to_sql_string(context, NameTag::name);
  //} else {
    return std::string(name.data);
  //}
}

template <typename Context>
auto name_to_sql_string(Context& /*context*/, const std::string_view& name) -> std::string {
  // TODO: if (NameTag::require_quotes) {
    //return quoted_name_to_sql_string(context, NameTag::name);
  //} else {
    return std::string(name);
  //}
}

// Has to be specialized for raw columns.
template <typename Context, typename Field>
auto select_field_to_sql_string(Context& context, const Field& f) -> std::string {
  return to_sql_name(context, f);
}

template <typename T, typename Context>
  requires(is_boolean_v<T>)
auto data_type_to_sql_string(Context&) -> std::string {
  return "BOOLEAN";
}

template <typename T, typename Context>
  requires(is_integral_v<T>)
auto data_type_to_sql_string(Context&) -> std::string {
  return "BIGINT";
}

template <typename T, typename Context>
  requires(is_unsigned_integral_v<T>)
auto data_type_to_sql_string(Context&) -> std::string {
  return "BIGINT UNSIGNED";
}

template <typename T, typename Context>
  requires(is_floating_point_v<T>)
auto data_type_to_sql_string(Context&) -> std::string {
  return "DOUBLE PRECISION";
}

template <typename T, typename Context>
  requires(is_text_v<T>)
auto data_type_to_sql_string(Context&) -> std::string {
  return "VARCHAR";
}

template <typename T, typename Context>
  requires(is_blob_v<T>)
auto data_type_to_sql_string(Context&) -> std::string {
  return "BLOB";
}

template <typename T, typename Context>
  requires(is_time_of_day_v<T>)
auto data_type_to_sql_string(Context&) -> std::string {
  return "TIME";
}

template <typename T, typename Context>
  requires(is_timestamp_v<T>)
auto data_type_to_sql_string(Context&) -> std::string {
  return "TIMESTAMP";
}

template <typename T, typename Context>
  requires(is_date_v<T>)
auto data_type_to_sql_string(Context&) -> std::string {
  return "DATE";
}

template <typename Context, typename Data>
auto dynamic_clause_to_sql_string(Context& context,
                                  std::string_view name,
                                  const Data& data) -> std::string {
  if constexpr (is_dynamic<Data>::value) {
    if (data.has_value()) {
      return std::format("{} {}", name, to_sql_string(context, data.value()));
    }
    return {};
  } else {
    return std::format("{} {}", name, to_sql_string(context, data));
  }
}

}  // namespace sqlpp
