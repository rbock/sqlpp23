#pragma once

/*
 * Copyright (c) 2026, Vesselin Atanasov
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

// The actual SQL representation of our C++ enum that will be stored in a
// nullable enum column
enum class shape { circle, square, triangle };

// Helper metafunctions that check if a variable is of our (possibly optional)
// enum type
template <typename T>
struct is_shape
    : public std::is_same<sqlpp::remove_optional_t<sqlpp::data_type_of_t<T>>,
                          shape> {};
template <typename T>
static constexpr bool is_shape_v = is_shape<T>::value;

// Helper function that converts shape to string
inline std::string_view shape_to_string(shape value) {
  switch (value) {
    case shape::circle:
      return "circle";
    case shape::square:
      return "square";
    case shape::triangle:
      return "triangle";
    default:
      throw std::runtime_error{std::format(
          "Cannot serialize a shape enum with non-enumerated value {}",
          std::to_underlying(value))};
  }
}

// Helper function that converts string to shape
inline shape string_to_shape(std::string_view value) {
  if (value == "circle") {
    return shape::circle;
  } else if (value == "square") {
    return shape::square;
  } else if (value == "triangle") {
    return shape::triangle;
  } else {
    throw std::runtime_error{
        std::format("Cannot convert '{}' to shape", value)};
  }
}

// Required to enable serialization of the enum
template <typename Context>
auto to_sql_string(Context& context, const shape& value) {
  return sqlpp::to_sql_string(context, shape_to_string(value));
}

// Required to enable reading of the enum from the database
template <typename Result>
void read_field(const Result& result, size_t field_index, shape& value) {
  std::string_view sv{};
  read_field(result, field_index, sv);
  value = string_to_shape(sv);
}

// Only needed if you want to use the enum as a paratemer in prepared statements
template <typename Statement>
void bind_parameter(Statement& stmt, size_t index, const shape& value) {
  bind_parameter(stmt, index, std::string{shape_to_string(value)});
}

namespace sqlpp {

// Required to enable the library to calculate the data type of any expression
// that uses our enum
template <>
struct data_type_of<shape> {
  using type = shape;
};

// Required to enable reading and deserialization of the enum from the database
template <>
struct result_data_type_of<shape> {
  using type = shape;
};

// Required to enable enum assignment during INSERT queries
template <typename L, typename R>
  requires(is_shape_v<L> && is_shape_v<R>)
struct values_are_assignable<L, R> : public std::true_type {};

// Only needed if you want to use the enum in ORDER BY clauses or compare it
// using ==, <, etc.
template <typename L, typename R>
  requires(is_shape_v<L> && is_shape_v<R>)
struct values_are_comparable<L, R> : public std::true_type {};

}  // namespace sqlpp
