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

#include <type_traits>

#include <sqlpp23/core/noop.h>
#include <sqlpp23/core/operator/enable_as.h>
#include <sqlpp23/core/reader.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
template <typename Lhs, typename Operator, typename Rhs>
struct bit_expression : public enable_as {
  constexpr bit_expression(Lhs lhs, Rhs rhs) : _lhs(std::move(lhs)), _rhs(std::move(rhs)) {}
  bit_expression(const bit_expression&) = default;
  bit_expression(bit_expression&&) = default;
  bit_expression& operator=(const bit_expression&) = default;
  bit_expression& operator=(bit_expression&&) = default;
  ~bit_expression() = default;

 private:
  friend reader_t;
  Lhs _lhs;
  Rhs _rhs;
};

template <typename Lhs, typename Operator, typename Rhs>
struct data_type_of<bit_expression<Lhs, Operator, Rhs>> {
  using type =
      std::conditional_t<sqlpp::is_optional<data_type_of_t<Lhs>>::value or
                             sqlpp::is_optional<data_type_of_t<Rhs>>::value,
                         std::optional<integral>,
                         integral>;
};

template <typename Lhs, typename Operator, typename Rhs>
struct nodes_of<bit_expression<Lhs, Operator, Rhs>> {
  using type = detail::type_vector<Lhs, Rhs>;
};

template <typename Lhs, typename Operator, typename Rhs>
struct requires_parentheses<bit_expression<Lhs, Operator, Rhs>>
    : public std::true_type {};

template <typename Context, typename Lhs, typename Operator, typename Rhs>
auto to_sql_string(Context& context, const bit_expression<Lhs, Operator, Rhs>& t)
    -> std::string {
  // Note: Temporary required to enforce parameter ordering.
  auto ret_val = operand_to_sql_string(context, read.lhs(t)) + Operator::symbol;
  return ret_val + operand_to_sql_string(context, read.rhs(t));
}

struct bit_and {
  static constexpr auto symbol = " & ";
};

template <typename Lhs, typename Rhs>
  requires(is_integral<Lhs>::value and is_integral<Rhs>::value)
constexpr auto operator&(Lhs lhs, Rhs rhs) -> bit_expression<Lhs, bit_and, Rhs> {
  return {std::move(lhs), std::move(rhs)};
}

struct bit_or {
  static constexpr auto symbol = " | ";
};

template <typename Lhs, typename Rhs>
  requires(is_integral<Lhs>::value and is_integral<Rhs>::value)
constexpr auto operator|(Lhs lhs, Rhs rhs) -> bit_expression<Lhs, bit_or, Rhs> {
  return {std::move(lhs), std::move(rhs)};
}

struct bit_xor {
  static constexpr auto symbol = " ^ ";
};

template <typename Lhs, typename Rhs>
  requires(is_integral<Lhs>::value and is_integral<Rhs>::value)
constexpr auto operator^(Lhs lhs, Rhs rhs) -> bit_expression<Lhs, bit_xor, Rhs> {
  return {std::move(lhs), std::move(rhs)};
}

struct bit_not {
  static constexpr auto symbol = "~";
};

template <typename Rhs>
  requires(is_integral<Rhs>::value)
constexpr auto operator~(Rhs rhs) -> bit_expression<noop, bit_not, Rhs> {
  return {{}, std::move(rhs)};
}

struct bit_shift_left {
  static constexpr auto symbol = " << ";
};

template <typename Lhs, typename Rhs>
  requires(is_integral<Lhs>::value and
           (is_integral<Rhs>::value or is_unsigned_integral<Rhs>::value))
constexpr auto operator<<(Lhs lhs, Rhs rhs) -> bit_expression<Lhs, bit_shift_left, Rhs> {
  return {std::move(lhs), std::move(rhs)};
}

struct bit_shift_right {
  static constexpr auto symbol = " >> ";
};

template <typename Lhs, typename Rhs>
  requires(is_integral<Lhs>::value and
           (is_integral<Rhs>::value or is_unsigned_integral<Rhs>::value))
constexpr auto operator>>(Lhs lhs, Rhs rhs) -> bit_expression<Lhs, bit_shift_right, Rhs> {
  return {std::move(lhs), std::move(rhs)};
}

}  // namespace sqlpp
