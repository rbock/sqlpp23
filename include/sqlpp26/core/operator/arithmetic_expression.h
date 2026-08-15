#pragma once

/*
Copyright (c) 2018, Roland Bock
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <utility>

#include <sqlpp26/core/function/concat.h>
#include <sqlpp26/core/noop.h>
#include <sqlpp26/core/operator/enable_as.h>
#include <sqlpp26/core/operator/enable_comparison.h>
#include <sqlpp26/core/reader.h>
#include <sqlpp26/core/type_traits.h>

namespace sqlpp {
struct plus {
  static constexpr auto symbol = " + ";
};

struct minus {
  static constexpr auto symbol = " - ";
};

struct multiplies {
  static constexpr auto symbol = " * ";
};

struct divides {
  static constexpr auto symbol = " / ";
};

struct negate {
  static constexpr auto symbol = "-";
};

struct modulus {
  static constexpr auto symbol = " % ";
};

template <typename Lhs, typename Operator, typename Rhs>
struct arithmetic_expression : public enable_as, public enable_comparison {
  arithmetic_expression() = delete;
  constexpr arithmetic_expression(Lhs lhs, Rhs rhs)
      : _lhs(std::move(lhs)), _rhs(std::move(rhs)) {}
  arithmetic_expression(const arithmetic_expression&) = default;
  arithmetic_expression(arithmetic_expression&&) = default;
  arithmetic_expression& operator=(const arithmetic_expression&) = default;
  arithmetic_expression& operator=(arithmetic_expression&&) = default;
  ~arithmetic_expression() = default;

 private:
  friend reader_t;
  Lhs _lhs;
  Rhs _rhs;
};

template <typename Operator, typename Lhs, typename Rhs>
struct arithmetic_data_type {
  consteval {
    throw std::domain_error("missing specialization for arithmetic_data_type");
  }
};

template <typename Operator, typename Lhs, typename Rhs>
using arithmetic_data_type_t =
    typename arithmetic_data_type<Operator, Lhs, Rhs>::type;

// Operator plus
template <typename Lhs, typename Rhs>
  requires(is_floating_point_v<Lhs> and is_floating_point_v<Rhs>)
struct arithmetic_data_type<plus, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_floating_point_v<Lhs> and is_integral_v<Rhs>)
struct arithmetic_data_type<plus, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_floating_point_v<Lhs> and is_unsigned_integral_v<Rhs>)
struct arithmetic_data_type<plus, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_floating_point_v<Lhs> and is_boolean_v<Rhs>)
struct arithmetic_data_type<plus, Lhs, Rhs> {
  using type = double;
};

template <typename Lhs, typename Rhs>
  requires(is_integral_v<Lhs> and is_floating_point_v<Rhs>)
struct arithmetic_data_type<plus, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_integral_v<Lhs> and is_integral_v<Rhs>)
struct arithmetic_data_type<plus, Lhs, Rhs> {
  using type = int64_t;
};
template <typename Lhs, typename Rhs>
  requires(is_integral_v<Lhs> and is_unsigned_integral_v<Rhs>)
struct arithmetic_data_type<plus, Lhs, Rhs> {
  using type = int64_t;
};
template <typename Lhs, typename Rhs>
  requires(is_integral_v<Lhs> and is_boolean_v<Rhs>)
struct arithmetic_data_type<plus, Lhs, Rhs> {
  using type = int64_t;
};

template <typename Lhs, typename Rhs>
  requires(is_unsigned_integral_v<Lhs> and is_floating_point_v<Rhs>)
struct arithmetic_data_type<plus, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_unsigned_integral_v<Lhs> and is_integral_v<Rhs>)
struct arithmetic_data_type<plus, Lhs, Rhs> {
  using type = int64_t;
};
template <typename Lhs, typename Rhs>
  requires(is_unsigned_integral_v<Lhs> and is_unsigned_integral_v<Rhs>)
struct arithmetic_data_type<plus, Lhs, Rhs> {
  using type = uint64_t;
};
template <typename Lhs, typename Rhs>
  requires(is_unsigned_integral_v<Lhs> and is_boolean_v<Rhs>)
struct arithmetic_data_type<plus, Lhs, Rhs> {
  using type = uint64_t;
};

template <typename Lhs, typename Rhs>
  requires(is_boolean_v<Lhs> and is_floating_point_v<Rhs>)
struct arithmetic_data_type<plus, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_boolean_v<Lhs> and is_integral_v<Rhs>)
struct arithmetic_data_type<plus, Lhs, Rhs> {
  using type = int64_t;
};
template <typename Lhs, typename Rhs>
  requires(is_boolean_v<Lhs> and is_unsigned_integral_v<Rhs>)
struct arithmetic_data_type<plus, Lhs, Rhs> {
  using type = uint64_t;
};
template <typename Lhs, typename Rhs>
  requires(is_boolean_v<Lhs> and is_boolean_v<Rhs>)
struct arithmetic_data_type<plus, Lhs, Rhs> {
  using type = uint64_t;
};

// Operator minus
template <typename Lhs, typename Rhs>
  requires(is_floating_point_v<Lhs> and is_floating_point_v<Rhs>)
struct arithmetic_data_type<minus, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_floating_point_v<Lhs> and is_integral_v<Rhs>)
struct arithmetic_data_type<minus, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_floating_point_v<Lhs> and is_unsigned_integral_v<Rhs>)
struct arithmetic_data_type<minus, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_floating_point_v<Lhs> and is_boolean_v<Rhs>)
struct arithmetic_data_type<minus, Lhs, Rhs> {
  using type = double;
};

template <typename Lhs, typename Rhs>
  requires(is_integral_v<Lhs> and is_floating_point_v<Rhs>)
struct arithmetic_data_type<minus, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_integral_v<Lhs> and is_integral_v<Rhs>)
struct arithmetic_data_type<minus, Lhs, Rhs> {
  using type = int64_t;
};
template <typename Lhs, typename Rhs>
  requires(is_integral_v<Lhs> and is_unsigned_integral_v<Rhs>)
struct arithmetic_data_type<minus, Lhs, Rhs> {
  using type = int64_t;
};
template <typename Lhs, typename Rhs>
  requires(is_integral_v<Lhs> and is_boolean_v<Rhs>)
struct arithmetic_data_type<minus, Lhs, Rhs> {
  using type = int64_t;
};

template <typename Lhs, typename Rhs>
  requires(is_unsigned_integral_v<Lhs> and is_floating_point_v<Rhs>)
struct arithmetic_data_type<minus, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_unsigned_integral_v<Lhs> and is_integral_v<Rhs>)
struct arithmetic_data_type<minus, Lhs, Rhs> {
  using type = int64_t;
};
template <typename Lhs, typename Rhs>
  requires(is_unsigned_integral_v<Lhs> and is_unsigned_integral_v<Rhs>)
struct arithmetic_data_type<minus, Lhs, Rhs> {
  using type = int64_t;
};
template <typename Lhs, typename Rhs>
  requires(is_unsigned_integral_v<Lhs> and is_boolean_v<Rhs>)
struct arithmetic_data_type<minus, Lhs, Rhs> {
  using type = int64_t;
};

template <typename Lhs, typename Rhs>
  requires(is_boolean_v<Lhs> and is_floating_point_v<Rhs>)
struct arithmetic_data_type<minus, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_boolean_v<Lhs> and is_integral_v<Rhs>)
struct arithmetic_data_type<minus, Lhs, Rhs> {
  using type = int64_t;
};
template <typename Lhs, typename Rhs>
  requires(is_boolean_v<Lhs> and is_unsigned_integral_v<Rhs>)
struct arithmetic_data_type<minus, Lhs, Rhs> {
  using type = int64_t;
};
template <typename Lhs, typename Rhs>
  requires(is_boolean_v<Lhs> and is_boolean_v<Rhs>)
struct arithmetic_data_type<minus, Lhs, Rhs> {
  using type = int64_t;
};

// Operator multiplies
template <typename Lhs, typename Rhs>
  requires(is_floating_point_v<Lhs> and is_floating_point_v<Rhs>)
struct arithmetic_data_type<multiplies, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_floating_point_v<Lhs> and is_integral_v<Rhs>)
struct arithmetic_data_type<multiplies, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_floating_point_v<Lhs> and is_unsigned_integral_v<Rhs>)
struct arithmetic_data_type<multiplies, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_floating_point_v<Lhs> and is_boolean_v<Rhs>)
struct arithmetic_data_type<multiplies, Lhs, Rhs> {
  using type = double;
};

template <typename Lhs, typename Rhs>
  requires(is_integral_v<Lhs> and is_floating_point_v<Rhs>)
struct arithmetic_data_type<multiplies, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_integral_v<Lhs> and is_integral_v<Rhs>)
struct arithmetic_data_type<multiplies, Lhs, Rhs> {
  using type = int64_t;
};
template <typename Lhs, typename Rhs>
  requires(is_integral_v<Lhs> and is_unsigned_integral_v<Rhs>)
struct arithmetic_data_type<multiplies, Lhs, Rhs> {
  using type = int64_t;
};
template <typename Lhs, typename Rhs>
  requires(is_integral_v<Lhs> and is_boolean_v<Rhs>)
struct arithmetic_data_type<multiplies, Lhs, Rhs> {
  using type = int64_t;
};

template <typename Lhs, typename Rhs>
  requires(is_unsigned_integral_v<Lhs> and is_floating_point_v<Rhs>)
struct arithmetic_data_type<multiplies, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_unsigned_integral_v<Lhs> and is_integral_v<Rhs>)
struct arithmetic_data_type<multiplies, Lhs, Rhs> {
  using type = int64_t;
};
template <typename Lhs, typename Rhs>
  requires(is_unsigned_integral_v<Lhs> and is_unsigned_integral_v<Rhs>)
struct arithmetic_data_type<multiplies, Lhs, Rhs> {
  using type = uint64_t;
};
template <typename Lhs, typename Rhs>
  requires(is_unsigned_integral_v<Lhs> and is_boolean_v<Rhs>)
struct arithmetic_data_type<multiplies, Lhs, Rhs> {
  using type = uint64_t;
};

template <typename Lhs, typename Rhs>
  requires(is_boolean_v<Lhs> and is_floating_point_v<Rhs>)
struct arithmetic_data_type<multiplies, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_boolean_v<Lhs> and is_integral_v<Rhs>)
struct arithmetic_data_type<multiplies, Lhs, Rhs> {
  using type = int64_t;
};
template <typename Lhs, typename Rhs>
  requires(is_boolean_v<Lhs> and is_unsigned_integral_v<Rhs>)
struct arithmetic_data_type<multiplies, Lhs, Rhs> {
  using type = uint64_t;
};
template <typename Lhs, typename Rhs>
  requires(is_boolean_v<Lhs> and is_boolean_v<Rhs>)
struct arithmetic_data_type<multiplies, Lhs, Rhs> {
  using type = bool;
};

// Operator divides
template <typename Lhs, typename Rhs>
  requires(is_floating_point_v<Lhs> and is_floating_point_v<Rhs>)
struct arithmetic_data_type<divides, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_floating_point_v<Lhs> and is_integral_v<Rhs>)
struct arithmetic_data_type<divides, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_floating_point_v<Lhs> and is_unsigned_integral_v<Rhs>)
struct arithmetic_data_type<divides, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_floating_point_v<Lhs> and is_boolean_v<Rhs>)
struct arithmetic_data_type<divides, Lhs, Rhs> {
  using type = double;
};

template <typename Lhs, typename Rhs>
  requires(is_integral_v<Lhs> and is_floating_point_v<Rhs>)
struct arithmetic_data_type<divides, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_integral_v<Lhs> and is_integral_v<Rhs>)
struct arithmetic_data_type<divides, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_integral_v<Lhs> and is_unsigned_integral_v<Rhs>)
struct arithmetic_data_type<divides, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_integral_v<Lhs> and is_boolean_v<Rhs>)
struct arithmetic_data_type<divides, Lhs, Rhs> {
  using type = double;
};

template <typename Lhs, typename Rhs>
  requires(is_unsigned_integral_v<Lhs> and is_floating_point_v<Rhs>)
struct arithmetic_data_type<divides, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_unsigned_integral_v<Lhs> and is_integral_v<Rhs>)
struct arithmetic_data_type<divides, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_unsigned_integral_v<Lhs> and is_unsigned_integral_v<Rhs>)
struct arithmetic_data_type<divides, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_unsigned_integral_v<Lhs> and is_boolean_v<Rhs>)
struct arithmetic_data_type<divides, Lhs, Rhs> {
  using type = double;
};

template <typename Lhs, typename Rhs>
  requires(is_boolean_v<Lhs> and is_floating_point_v<Rhs>)
struct arithmetic_data_type<divides, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_boolean_v<Lhs> and is_integral_v<Rhs>)
struct arithmetic_data_type<divides, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_boolean_v<Lhs> and is_unsigned_integral_v<Rhs>)
struct arithmetic_data_type<divides, Lhs, Rhs> {
  using type = double;
};
template <typename Lhs, typename Rhs>
  requires(is_boolean_v<Lhs> and is_boolean_v<Rhs>)
struct arithmetic_data_type<divides, Lhs, Rhs> {
  using type = double;
};

// Operator negate
template <typename Rhs>
  requires(is_floating_point_v<Rhs>)
struct arithmetic_data_type<negate, no_value_t, Rhs> {
  using type = double;
};
template <typename Rhs>
  requires(is_integral_v<Rhs>)
struct arithmetic_data_type<negate, no_value_t, Rhs> {
  using type = int64_t;
};
template <typename Rhs>
  requires(is_unsigned_integral_v<Rhs>)
struct arithmetic_data_type<negate, no_value_t, Rhs> {
  using type = int64_t;
};
template <typename Rhs>
  requires(is_boolean_v<Rhs>)
struct arithmetic_data_type<negate, no_value_t, Rhs> {
  using type = int64_t;
};

// Operator modulus
template <typename Lhs, typename Rhs>
  requires(is_integral_v<Lhs> and is_integral_v<Rhs>)
struct arithmetic_data_type<modulus, Lhs, Rhs> {
  using type = uint64_t;
};
template <typename Lhs, typename Rhs>
  requires(is_integral_v<Lhs> and is_unsigned_integral_v<Rhs>)
struct arithmetic_data_type<modulus, Lhs, Rhs> {
  using type = uint64_t;
};

template <typename Lhs, typename Rhs>
  requires(is_unsigned_integral_v<Lhs> and is_integral_v<Rhs>)
struct arithmetic_data_type<modulus, Lhs, Rhs> {
  using type = uint64_t;
};
template <typename Lhs, typename Rhs>
  requires(is_unsigned_integral_v<Lhs> and is_unsigned_integral_v<Rhs>)
struct arithmetic_data_type<modulus, Lhs, Rhs> {
  using type = uint64_t;
};

// Handle optional types
template <typename Operator, typename Lhs, typename Rhs>
struct arithmetic_data_type<Operator, std::optional<Lhs>, Rhs> {
  using type = std::optional<arithmetic_data_type_t<Operator, Lhs, Rhs>>;
};

template <typename Operator, typename Lhs, typename Rhs>
struct arithmetic_data_type<Operator, Lhs, std::optional<Rhs>> {
  using type = std::optional<arithmetic_data_type_t<Operator, Lhs, Rhs>>;
};

template <typename Operator, typename Lhs, typename Rhs>
struct arithmetic_data_type<Operator, std::optional<Lhs>, std::optional<Rhs>> {
  using type = std::optional<arithmetic_data_type_t<Operator, Lhs, Rhs>>;
};

template <typename Operator, typename Lhs, typename Rhs>
struct data_type_of<arithmetic_expression<Lhs, Operator, Rhs>>
    : public arithmetic_data_type<Operator,
                                  data_type_of_t<Lhs>,
                                  data_type_of_t<Rhs>> {};

template <typename Lhs, typename Operator, typename Rhs>
struct nodes_of<arithmetic_expression<Lhs, Operator, Rhs>> {
  using type = detail::type_vector<Lhs, Rhs>;
};

template <typename Lhs, typename Operator, typename Rhs>
struct requires_parentheses<arithmetic_expression<Lhs, Operator, Rhs>>
    : public std::true_type {};

template <typename Context, typename Lhs, typename Operator, typename Rhs>
auto to_sql_string(Context& context,
                   const arithmetic_expression<Lhs, Operator, Rhs>& t)
    -> std::string {
  // Note: Temporary required to enforce parameter ordering.
  auto ret_val = operand_to_sql_string(context, read.lhs(t)) + Operator::symbol;
  return ret_val + operand_to_sql_string(context, read.rhs(t));
}

template <typename Lhs, typename Rhs>
  requires(is_numeric<Lhs>::value and is_numeric<Rhs>::value)
constexpr auto operator+(Lhs lhs, Rhs rhs)
    -> arithmetic_expression<Lhs, plus, Rhs> {
  return {std::move(lhs), std::move(rhs)};
}

template <typename Lhs, typename Rhs>
  requires(is_text<Lhs>::value and is_text<Rhs>::value)
constexpr auto operator+(Lhs lhs, Rhs rhs) -> decltype(concat(lhs, rhs)) {
  return concat(std::move(lhs), std::move(rhs));
}

template <typename Lhs, typename Rhs>
  requires(is_numeric<Lhs>::value and is_numeric<Rhs>::value)
constexpr auto operator-(Lhs lhs, Rhs rhs)
    -> arithmetic_expression<Lhs, minus, Rhs> {
  return {std::move(lhs), std::move(rhs)};
}

template <typename Lhs, typename Rhs>
  requires(is_numeric<Lhs>::value and is_numeric<Rhs>::value)
constexpr auto operator*(Lhs lhs, Rhs rhs)
    -> arithmetic_expression<Lhs, multiplies, Rhs> {
  return {std::move(lhs), std::move(rhs)};
}

template <typename Lhs, typename Rhs>
  requires(is_numeric<Lhs>::value and is_numeric<Rhs>::value)
constexpr auto operator/(Lhs lhs, Rhs rhs)
    -> arithmetic_expression<Lhs, divides, Rhs> {
  return {std::move(lhs), std::move(rhs)};
}

template <typename Rhs>
  requires(is_numeric<Rhs>::value)
constexpr auto operator-(Rhs rhs) -> arithmetic_expression<noop, negate, Rhs> {
  return {{}, std::move(rhs)};
}

template <typename Lhs, typename Rhs>
  requires((is_integral<Lhs>::value or is_unsigned_integral<Lhs>::value) and
           (is_integral<Rhs>::value or is_unsigned_integral<Rhs>::value))
constexpr auto operator%(Lhs lhs, Rhs rhs)
    -> arithmetic_expression<Lhs, modulus, Rhs> {
  return {std::move(lhs), std::move(rhs)};
}

}  // namespace sqlpp
