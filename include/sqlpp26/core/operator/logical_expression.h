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

#include <sqlpp26/core/concepts.h>
#include <sqlpp26/core/logic.h>
#include <sqlpp26/core/noop.h>
#include <sqlpp26/core/operator/enable_as.h>
#include <sqlpp26/core/query/dynamic.h>
#include <sqlpp26/core/reader.h>
#include <sqlpp26/core/to_sql_string.h>
#include <sqlpp26/core/tuple_to_sql_string.h>
#include <sqlpp26/core/type_traits.h>

namespace sqlpp {
struct logical_and {
  static constexpr auto symbol = " AND ";
};

struct logical_or {
  static constexpr auto symbol = " OR ";
};

template <typename Operator, typename... Expressions>
struct logical_expression : public enable_as {
  logical_expression() = delete;
  constexpr logical_expression(std::tuple<Expressions...> expressions)
      : _expressions(std::move(expressions)) {}
  logical_expression(const logical_expression&) = default;
  logical_expression(logical_expression&&) = default;
  logical_expression& operator=(const logical_expression&) = default;
  logical_expression& operator=(logical_expression&&) = default;
  ~logical_expression() = default;

 private:
  friend reader_t;
  std::tuple<Expressions...> _expressions;
};

template <typename Operator, typename... Expressions>
struct data_type_of<logical_expression<Operator, Expressions...>>
    : std::conditional<logic::any<sqlpp::is_optional<data_type_of_t<
                           remove_dynamic_t<Expressions>>>::value...>::value,
                       std::optional<boolean>,
                       boolean> {};

template <typename Operator, typename... Expressions>
struct nodes_of<logical_expression<Operator, Expressions...>> {
  using type = detail::type_vector<Expressions...>;
};

template <typename Operator, typename... Expressions>
struct requires_parentheses<logical_expression<Operator, Expressions...>>
    : public std::true_type {};

template <typename Context, typename Operator, typename... Expressions>
auto to_sql_string(Context& context,
                   const logical_expression<Operator, Expressions...>& t)
    -> std::string {
  return tuple_to_sql_string(context, read.expressions(t),
                             tuple_operand_no_dynamic{Operator::symbol});
}

template <StaticBoolean Lhs, DynamicBoolean Rhs>
constexpr auto operator and(Lhs lhs, Rhs rhs)
    -> logical_expression<logical_and, Lhs, Rhs> {
  return {std::make_tuple(std::move(lhs), std::move(rhs))};
}

// Chain AND expressions
template <typename... Expressions, DynamicBoolean Rhs>
constexpr auto operator and(logical_expression<logical_and, Expressions...> lhs,
                            Rhs rhs)
    -> logical_expression<logical_and, Expressions..., Rhs> {
  return {std::tuple_cat(std::move(read.non_const_expressions(lhs)),
                         std::make_tuple(std::move(rhs)))};
}

template <StaticBoolean Lhs, DynamicBoolean Rhs>
constexpr auto operator||(Lhs lhs, Rhs rhs)
    -> logical_expression<logical_or, Lhs, Rhs> {
  return {std::make_tuple(std::move(lhs), std::move(rhs))};
}

// Chain OR expressions
template <typename... Expressions, DynamicBoolean Rhs>
constexpr auto operator or(logical_expression<logical_or, Expressions...> lhs,
                           Rhs rhs)
    -> logical_expression<logical_or, Expressions..., Rhs> {
  return {std::tuple_cat(std::move(read.non_const_expressions(lhs)),
                         std::make_tuple(std::move(rhs)))};
}

struct logical_not {
  static constexpr auto symbol = "NOT ";
};

template <StaticBoolean Rhs>
constexpr auto operator!(Rhs rhs)
    -> logical_expression<logical_not, noop, Rhs> {
  return {std::make_tuple(noop{}, std::move(rhs))};
}

}  // namespace sqlpp
