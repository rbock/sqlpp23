#pragma once

/*
Copyright (c) 2017 - 2018, Roland Bock
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

#include <sqlpp23/core/basic/column_fwd.h>
#include <sqlpp23/core/default_value.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
template <typename L, typename Operator, typename R>
struct assign_expression {
  constexpr assign_expression(L l, R r) : _l(std::move(l)), _r(std::move(r)) {}
  assign_expression(const assign_expression&) = default;
  assign_expression(assign_expression&&) = default;
  assign_expression& operator=(const assign_expression&) = default;
  assign_expression& operator=(assign_expression&&) = default;
  ~assign_expression() = default;

  L _l;
  R _r;
};

template <typename L, typename Operator, typename R>
auto get_rhs(assign_expression<L, Operator, R> e) -> R {
  return e._r;
}

template <typename L, typename Operator, typename R>
auto get_rhs(dynamic_t<assign_expression<L, Operator, R>> e) -> dynamic_t<R> {
  if (e.has_value()) {
    return {e.value()._r};
  }
  return {std::nullopt};
}

template <typename L, typename R>
constexpr bool are_correct_assignment_args =
    not is_const<L>::value and values_are_assignable<L, R>::value and
    (can_be_null<L>::value or not can_be_null<R>::value);

template <typename L>
constexpr bool are_correct_assignment_args<L, default_value_t> =
    not is_const<L>::value and has_default<L>::value;

template <typename L, typename Operator, typename R>
struct is_assignment<assign_expression<L, Operator, R>>
    : public std::true_type {};

template <typename L, typename Operator, typename R>
struct nodes_of<assign_expression<L, Operator, R>> {
  using type = detail::type_vector<L, R>;
};

template <typename L, typename Operator, typename R>
struct lhs<assign_expression<L, Operator, R>> {
  using type = L;
};

template <typename L, typename Operator, typename R>
struct rhs<assign_expression<L, Operator, R>> {
  using type = R;
};

template <typename Context, typename L, typename Operator, typename R>
auto to_sql_string(Context& context, const assign_expression<L, Operator, R>& t)
    -> std::string {
  return to_sql_string(context, simple_column(t._l)) + Operator::symbol +
         operand_to_sql_string(context, t._r);
}

struct op_assign {
  static constexpr auto symbol = " = ";
};

template <typename _Table, typename ColumnSpec, typename R>
  requires(are_correct_assignment_args<column_t<_Table, ColumnSpec>, R>)
constexpr auto assign(column_t<_Table, ColumnSpec> column, R value)
    -> assign_expression<column_t<_Table, ColumnSpec>, op_assign, R> {
  return {std::move(column), std::move(value)};
}

}  // namespace sqlpp
