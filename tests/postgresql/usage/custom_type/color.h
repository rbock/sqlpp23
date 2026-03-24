#pragma once

/*
 * Copyright (c) 2026, Matthijs Möhlmann
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

#include <string>
#include <string_view>

#include <sqlpp23/core/database/exception.h>
#include <sqlpp23/core/type_traits.h>

// ---------------------------------------------------------------------------
// Color — C++ enum class mapped to a PostgreSQL native ENUM type.
//
// PostgreSQL ENUM values are transmitted as text on the wire. to_sql_string,
// read_field, and bind_parameter handle the string conversion transparently,
// so call sites work with Color directly.
// ---------------------------------------------------------------------------

enum class Color { red, green, blue };

// Predicate: matches any expression whose data type is Color (bare Color
// values, columns with data_type = Color, etc.).
template <typename T>
struct is_color : public std::is_same<sqlpp::data_type_of_t<T>, Color> {};
template <typename T>
inline constexpr bool is_color_v = is_color<T>::value;

// Conversion helpers — keep them close to the type so read_field and
// to_sql_string can share them without duplication.
inline std::string_view color_to_string(Color c) {
  switch (c) {
    case Color::red:
      return "red";
    case Color::green:
      return "green";
    case Color::blue:
      return "blue";
  }
  throw sqlpp::exception{"Unknown Color value"};
}

inline Color color_from_string(std::string_view sv) {
  if (sv == "red")
    return Color::red;
  if (sv == "green")
    return Color::green;
  if (sv == "blue")
    return Color::blue;
  throw sqlpp::exception{std::string{"Unknown Color string: "} +
                         std::string{sv}};
}

// Serialize Color to a quoted SQL string literal.
template <typename Context>
auto to_sql_string(Context& context, const Color& c) {
  using sqlpp::to_sql_string;
  return to_sql_string(context, color_to_string(c));
}

// Read a Color from a result row field (via string_view).
template <typename Result>
void read_field(const Result& result, size_t field_index, Color& c) {
  std::string_view sv;
  read_field(result, field_index, sv);
  c = color_from_string(sv);
}

// Bind a Color as a prepared statement parameter (via std::string, since
// PostgreSQL's bind_parameter only has a const std::string& text overload).
template <typename Statement>
void bind_parameter(Statement& stmt, size_t index, const Color& c) {
  bind_parameter(stmt, index, std::string{color_to_string(c)});
}

// ---------------------------------------------------------------------------
// namespace sqlpp — type system glue
// ---------------------------------------------------------------------------

namespace sqlpp {

// Color is its own data type tag, giving it a distinct identity from all
// other types (including other enums and sqlpp::text).
template <>
struct data_type_of<Color> {
  using type = Color;
};

// Result rows hold Color directly.
template <>
struct result_data_type_of<Color> {
  using type = Color;
};

// Prepared statement parameter structs hold Color directly.
template <>
struct parameter_value<Color> {
  using type = Color;
};

// Only Color can be assigned to a Color column (SET / INSERT).
template <typename L, typename R>
  requires(is_color_v<L> and is_color_v<R>)
struct values_are_assignable<L, R> : public std::true_type {};

// Only Color can be compared to a Color column (WHERE, ORDER BY).
template <typename L, typename R>
  requires(is_color_v<L> and is_color_v<R>)
struct values_are_comparable<L, R> : public std::true_type {};

}  // namespace sqlpp
