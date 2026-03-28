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

#include <sqlpp23/core/concepts.h>
#include <sqlpp23/core/noop.h>
#include <sqlpp23/core/operator/enable_as.h>
#include <sqlpp23/core/query/dynamic.h>
#include <sqlpp23/core/reader.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
struct logical_and {
  static constexpr auto symbol = " AND ";
};

struct logical_or {
  static constexpr auto symbol = " OR ";
};

template <typename Lhs, typename Operator, typename Rhs>
struct logical_expression : public enable_as {
  logical_expression() = delete;
  constexpr logical_expression(Lhs lhs, Rhs rhs)
      : _lhs(std::move(lhs)), _rhs(std::move(rhs)) {}
  logical_expression(const logical_expression&) = default;
  logical_expression(logical_expression&&) = default;
  logical_expression& operator=(const logical_expression&) = default;
  logical_expression& operator=(logical_expression&&) = default;
  ~logical_expression() = default;

 private:
  friend reader_t;
  Lhs _lhs;
  Rhs _rhs;
};

template <typename Lhs, typename Operator, typename Rhs>
struct data_type_of<logical_expression<Lhs, Operator, Rhs>>
    : std::conditional<
          sqlpp::is_optional<data_type_of_t<Lhs>>::value or
              sqlpp::is_optional<data_type_of_t<remove_dynamic_t<Rhs>>>::value,
          std::optional<boolean>,
          boolean> {};

template <typename Lhs, typename Operator, typename Rhs>
struct nodes_of<logical_expression<Lhs, Operator, Rhs>> {
  using type = detail::type_vector<Lhs, Rhs>;
};

template <typename Lhs, typename Operator, typename Rhs>
struct requires_parentheses<logical_expression<Lhs, Operator, Rhs>>
    : public std::true_type {};

template <typename Context, typename Lhs, typename Operator, typename Rhs>
auto to_sql_string(Context& context,
                   const logical_expression<Lhs, Operator, Rhs>& t)
    -> std::string {
  // Note: Temporary required to enforce parameter ordering.
  auto ret_val = operand_to_sql_string(context, read.lhs(t)) + Operator::symbol;
  return ret_val + operand_to_sql_string(context, read.rhs(t));
}

template <typename Context, typename Lhs, typename Operator, typename Rhs>
auto to_sql_string(Context& context,
                   const logical_expression<Lhs, Operator, dynamic_t<Rhs>>& t)
    -> std::string {
  if (read.rhs(t).has_value()) {
    // Note: Temporary required to enforce parameter ordering.
    auto ret_val = operand_to_sql_string(context, read.lhs(t)) + Operator::symbol;
    return ret_val + operand_to_sql_string(context, read.rhs(t).value());
  }

  // If the dynamic part is inactive ignore it.
  return to_sql_string(context, read.lhs(t));
}

template <typename Context,
          typename Lhs,
          typename Operator,
          typename R1,
          typename R2>
auto to_sql_string(
    Context& context,
    const logical_expression<logical_expression<Lhs, Operator, R1>,
                             Operator,
                             R2>& t) -> std::string {
  // Note: Temporary required to enforce parameter ordering.
  auto ret_val = to_sql_string(context, read.lhs(t)) + Operator::symbol;
  return ret_val + operand_to_sql_string(context, read.rhs(t));
}

template <typename Context,
          typename Lhs,
          typename Operator,
          typename R1,
          typename R2>
auto to_sql_string(
    Context& context,
    const logical_expression<logical_expression<Lhs, Operator, R1>,
                             Operator,
                             dynamic_t<R2>>& t) -> std::string {
  if (read.rhs(t).has_value()) {
    // Note: Temporary required to enforce parameter ordering.
    auto ret_val = to_sql_string(context, read.lhs(t)) + Operator::symbol;
    return ret_val + operand_to_sql_string(context, read.rhs(read.rhs(t).value()));
  }

  // If the dynamic part is inactive ignore it.
  return to_sql_string(context, read.lhs(t));
}

template <StaticBoolean Lhs, DynamicBoolean Rhs>
constexpr auto operator and(Lhs lhs, Rhs rhs)
    -> logical_expression<Lhs, logical_and, Rhs> {
  return {std::move(lhs), std::move(rhs)};
}

template <StaticBoolean Lhs, DynamicBoolean Rhs>
constexpr auto operator||(Lhs lhs, Rhs rhs)
    -> logical_expression<Lhs, logical_or, Rhs> {
  return {std::move(lhs), std::move(rhs)};
}

struct logical_not {
  static constexpr auto symbol = "NOT ";
};

template <StaticBoolean Rhs>
constexpr auto operator!(Rhs rhs)
    -> logical_expression<noop, logical_not, Rhs> {
  return {{}, std::move(rhs)};
}

}  // namespace sqlpp
