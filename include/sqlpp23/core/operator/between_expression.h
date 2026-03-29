#pragma once

/*
Copyright (c) 2024, Roland Bock
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

#include <sqlpp23/core/operator/enable_as.h>
#include <sqlpp23/core/operator/enable_comparison.h>
#include <sqlpp23/core/reader.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
template <typename Expression, typename Lhs, typename Rhs>
struct between_expression : public enable_as, public enable_comparison {
  constexpr between_expression(Expression lhs, Lhs low, Rhs high)
      : _expression(std::move(lhs)), _lhs(std::move(low)), _rhs(std::move(high)) {}
  between_expression(const between_expression&) = default;
  between_expression(between_expression&&) = default;
  between_expression& operator=(const between_expression&) = default;
  between_expression& operator=(between_expression&&) = default;
  ~between_expression() = default;

 private:
  friend reader_t;
  Expression _expression;
  Lhs _lhs;
  Rhs _rhs;
};

template <typename Expression, typename Lhs, typename Rhs>
struct data_type_of<between_expression<Expression, Lhs, Rhs>>
    : public std::conditional<
          sqlpp::is_optional<data_type_of_t<Expression>>::value or
              sqlpp::is_optional<data_type_of_t<Lhs>>::value or
              sqlpp::is_optional<data_type_of_t<Rhs>>::value,
          std::optional<boolean>,
          boolean> {};

template <typename Expression, typename Lhs, typename Rhs>
struct nodes_of<between_expression<Expression, Lhs, Rhs>> {
  using type = detail::type_vector<Expression, Lhs, Rhs>;
};

template <typename Expression, typename Lhs, typename Rhs>
struct requires_parentheses<between_expression<Expression, Lhs, Rhs>>
    : public std::true_type {};

template <typename Context, typename Expression, typename Lhs, typename Rhs>
auto to_sql_string(Context& context, const between_expression<Expression, Lhs, Rhs>& t)
    -> std::string {
  // Note: Temporary required to enforce parameter ordering.
  auto ret_val = operand_to_sql_string(context, read.expression(t)) + " BETWEEN ";
  ret_val += operand_to_sql_string(context, read.lhs(t)) + " AND ";
  return ret_val + operand_to_sql_string(context, read.rhs(t));
}

}  // namespace sqlpp
