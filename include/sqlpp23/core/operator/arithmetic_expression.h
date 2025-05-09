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

#include <sqlpp23/core/function/concat.h>
#include <sqlpp23/core/noop.h>
#include <sqlpp23/core/operator/enable_as.h>
#include <sqlpp23/core/operator/enable_comparison.h>
#include <sqlpp23/core/type_traits.h>

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

template <typename L, typename Operator, typename R>
struct arithmetic_expression : public enable_as, public enable_comparison {
  arithmetic_expression() = delete;
  constexpr arithmetic_expression(L l, R r) : _l(l), _r(r) {}
  arithmetic_expression(const arithmetic_expression&) = default;
  arithmetic_expression(arithmetic_expression&&) = default;
  arithmetic_expression& operator=(const arithmetic_expression&) = default;
  arithmetic_expression& operator=(arithmetic_expression&&) = default;
  ~arithmetic_expression() = default;

  L _l;
  R _r;
};

// L and R are expected to be numeric value types (boolean, integral,
// unsigned_integral, or floating_point).
template <typename Operator, typename L, typename R>
struct arithmetic_data_type {
  using type = numeric;
};

template <typename Operator, typename L, typename R>
using arithmetic_data_type_t =
    typename arithmetic_data_type<Operator, L, R>::type;

#define SQLPP_ARITHMETIC_DATA_TYPE(Op, Left, Right, DataType) \
  template <>                                                   \
  struct arithmetic_data_type<Op, Left, Right> {               \
    using type = DataType;                                     \
  };

// Operator plus
SQLPP_ARITHMETIC_DATA_TYPE(plus,
                            floating_point,
                            floating_point,
                            floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(plus, floating_point, integral, floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(plus,
                            floating_point,
                            unsigned_integral,
                            floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(plus, floating_point, boolean, floating_point);

SQLPP_ARITHMETIC_DATA_TYPE(plus, integral, floating_point, floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(plus, integral, integral, integral);
SQLPP_ARITHMETIC_DATA_TYPE(plus, integral, unsigned_integral, integral);
SQLPP_ARITHMETIC_DATA_TYPE(plus, integral, boolean, integral);

SQLPP_ARITHMETIC_DATA_TYPE(plus,
                            unsigned_integral,
                            floating_point,
                            floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(plus, unsigned_integral, integral, integral);
SQLPP_ARITHMETIC_DATA_TYPE(plus,
                            unsigned_integral,
                            unsigned_integral,
                            unsigned_integral);
SQLPP_ARITHMETIC_DATA_TYPE(plus,
                            unsigned_integral,
                            boolean,
                            unsigned_integral);

SQLPP_ARITHMETIC_DATA_TYPE(plus, boolean, floating_point, floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(plus, boolean, integral, integral);
SQLPP_ARITHMETIC_DATA_TYPE(plus,
                            boolean,
                            unsigned_integral,
                            unsigned_integral);
SQLPP_ARITHMETIC_DATA_TYPE(plus, boolean, boolean, unsigned_integral);

// Operator minus
SQLPP_ARITHMETIC_DATA_TYPE(minus,
                            floating_point,
                            floating_point,
                            floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(minus, floating_point, integral, floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(minus,
                            floating_point,
                            unsigned_integral,
                            floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(minus, floating_point, boolean, floating_point);

SQLPP_ARITHMETIC_DATA_TYPE(minus, integral, floating_point, floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(minus, integral, integral, integral);
SQLPP_ARITHMETIC_DATA_TYPE(minus, integral, unsigned_integral, integral);
SQLPP_ARITHMETIC_DATA_TYPE(minus, integral, boolean, integral);

SQLPP_ARITHMETIC_DATA_TYPE(minus,
                            unsigned_integral,
                            floating_point,
                            floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(minus, unsigned_integral, integral, integral);
SQLPP_ARITHMETIC_DATA_TYPE(minus,
                            unsigned_integral,
                            unsigned_integral,
                            integral);
SQLPP_ARITHMETIC_DATA_TYPE(minus, unsigned_integral, boolean, integral);

SQLPP_ARITHMETIC_DATA_TYPE(minus, boolean, floating_point, floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(minus, boolean, integral, integral);
SQLPP_ARITHMETIC_DATA_TYPE(minus, boolean, unsigned_integral, integral);
SQLPP_ARITHMETIC_DATA_TYPE(minus, boolean, boolean, integral);

// Operator multiplies
SQLPP_ARITHMETIC_DATA_TYPE(multiplies,
                            floating_point,
                            floating_point,
                            floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(multiplies,
                            floating_point,
                            integral,
                            floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(multiplies,
                            floating_point,
                            unsigned_integral,
                            floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(multiplies,
                            floating_point,
                            boolean,
                            floating_point);

SQLPP_ARITHMETIC_DATA_TYPE(multiplies,
                            integral,
                            floating_point,
                            floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(multiplies, integral, integral, integral);
SQLPP_ARITHMETIC_DATA_TYPE(multiplies, integral, unsigned_integral, integral);
SQLPP_ARITHMETIC_DATA_TYPE(multiplies, integral, boolean, integral);

SQLPP_ARITHMETIC_DATA_TYPE(multiplies,
                            unsigned_integral,
                            floating_point,
                            floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(multiplies, unsigned_integral, integral, integral);
SQLPP_ARITHMETIC_DATA_TYPE(multiplies,
                            unsigned_integral,
                            unsigned_integral,
                            unsigned_integral);
SQLPP_ARITHMETIC_DATA_TYPE(multiplies,
                            unsigned_integral,
                            boolean,
                            unsigned_integral);

SQLPP_ARITHMETIC_DATA_TYPE(multiplies,
                            boolean,
                            floating_point,
                            floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(multiplies, boolean, integral, integral);
SQLPP_ARITHMETIC_DATA_TYPE(multiplies,
                            boolean,
                            unsigned_integral,
                            unsigned_integral);
SQLPP_ARITHMETIC_DATA_TYPE(multiplies, boolean, boolean, boolean);

// Operator divides
SQLPP_ARITHMETIC_DATA_TYPE(divides,
                            floating_point,
                            floating_point,
                            floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(divides, floating_point, integral, floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(divides,
                            floating_point,
                            unsigned_integral,
                            floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(divides, floating_point, boolean, floating_point);

SQLPP_ARITHMETIC_DATA_TYPE(divides, integral, floating_point, floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(divides, integral, integral, floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(divides,
                            integral,
                            unsigned_integral,
                            floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(divides, integral, boolean, floating_point);

SQLPP_ARITHMETIC_DATA_TYPE(divides,
                            unsigned_integral,
                            floating_point,
                            floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(divides,
                            unsigned_integral,
                            integral,
                            floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(divides,
                            unsigned_integral,
                            unsigned_integral,
                            floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(divides,
                            unsigned_integral,
                            boolean,
                            floating_point);

SQLPP_ARITHMETIC_DATA_TYPE(divides, boolean, floating_point, floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(divides, boolean, integral, floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(divides,
                            boolean,
                            unsigned_integral,
                            floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(divides, boolean, boolean, floating_point);

// Operator negate
SQLPP_ARITHMETIC_DATA_TYPE(negate, no_value_t, floating_point, floating_point);
SQLPP_ARITHMETIC_DATA_TYPE(negate, no_value_t, integral, integral);
SQLPP_ARITHMETIC_DATA_TYPE(negate, no_value_t, unsigned_integral, integral);
SQLPP_ARITHMETIC_DATA_TYPE(negate, no_value_t, boolean, integral);

// Operator modulus
SQLPP_ARITHMETIC_DATA_TYPE(modulus, integral, integral, unsigned_integral);
SQLPP_ARITHMETIC_DATA_TYPE(modulus,
                            integral,
                            unsigned_integral,
                            unsigned_integral);

SQLPP_ARITHMETIC_DATA_TYPE(modulus,
                            unsigned_integral,
                            integral,
                            unsigned_integral);
SQLPP_ARITHMETIC_DATA_TYPE(modulus,
                            unsigned_integral,
                            unsigned_integral,
                            unsigned_integral);

#undef SQLPP_ARITHMETIC_DATA_TYPE

// Handle optional types
template <typename Operator, typename L, typename R>
struct arithmetic_data_type<Operator, std::optional<L>, R> {
  using type = std::optional<arithmetic_data_type_t<Operator, L, R>>;
};

template <typename Operator, typename L, typename R>
struct arithmetic_data_type<Operator, L, std::optional<R>> {
  using type = std::optional<arithmetic_data_type_t<Operator, L, R>>;
};

template <typename Operator, typename L, typename R>
struct arithmetic_data_type<Operator, std::optional<L>, std::optional<R>> {
  using type = std::optional<arithmetic_data_type_t<Operator, L, R>>;
};

template <typename Operator, typename L, typename R>
struct data_type_of<arithmetic_expression<L, Operator, R>>
    : public arithmetic_data_type<Operator,
                                   data_type_of_t<L>,
                                   data_type_of_t<R>> {};

template <typename L, typename Operator, typename R>
struct nodes_of<arithmetic_expression<L, Operator, R>> {
  using type = detail::type_vector<L, R>;
};

template <typename L, typename Operator, typename R>
struct requires_parentheses<arithmetic_expression<L, Operator, R>>
    : public std::true_type {};

template <typename Context, typename L, typename Operator, typename R>
auto to_sql_string(Context& context,
                   const arithmetic_expression<L, Operator, R>& t)
    -> std::string {
  // Note: Temporary required to enforce parameter ordering.
  auto ret_val = operand_to_sql_string(context, t._l) + Operator::symbol;
  return ret_val + operand_to_sql_string(context, t._r);
}

template <typename L, typename R>
  requires(is_numeric<L>::value and is_numeric<R>::value)
constexpr auto operator+(L l, R r) -> arithmetic_expression<L, plus, R> {
  return {std::move(l), std::move(r)};
}

template <typename L, typename R>
  requires(is_text<L>::value and is_text<R>::value)
constexpr auto operator+(L l, R r) -> decltype(concat(l, r)) {
  return concat(std::move(l), std::move(r));
}

template <typename L, typename R>
  requires(is_numeric<L>::value and is_numeric<R>::value)
constexpr auto operator-(L l, R r) -> arithmetic_expression<L, minus, R> {
  return {std::move(l), std::move(r)};
}

template <typename L, typename R>
  requires(is_numeric<L>::value and is_numeric<R>::value)
constexpr auto operator*(L l, R r) -> arithmetic_expression<L, multiplies, R> {
  return {std::move(l), std::move(r)};
}

template <typename L, typename R>
  requires(is_numeric<L>::value and is_numeric<R>::value)
constexpr auto operator/(L l, R r) -> arithmetic_expression<L, divides, R> {
  return {std::move(l), std::move(r)};
}

template <typename R>
  requires(is_numeric<R>::value)
constexpr auto operator-(R r) -> arithmetic_expression<noop, negate, R> {
  return {{}, std::move(r)};
}

template <typename L, typename R>
  requires((is_integral<L>::value or is_unsigned_integral<L>::value) and
           (is_integral<R>::value or is_unsigned_integral<R>::value))
constexpr auto operator%(L l, R r) -> arithmetic_expression<L, modulus, R> {
  return {std::move(l), std::move(r)};
}

}  // namespace sqlpp
