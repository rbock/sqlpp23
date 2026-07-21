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
#include <sqlpp23/core/reader.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
template <typename Lhs, typename Operator, typename Rhs>
struct assign_expression {
  constexpr assign_expression(Lhs lhs, Rhs rhs) : _lhs(std::move(lhs)), _rhs(std::move(rhs)) {}
  assign_expression(const assign_expression&) = default;
  assign_expression(assign_expression&&) = default;
  assign_expression& operator=(const assign_expression&) = default;
  assign_expression& operator=(assign_expression&&) = default;
  ~assign_expression() = default;

 private:
  friend reader_t;
  Lhs _lhs;
  Rhs _rhs;
};

template <typename Lhs, typename Operator, typename Rhs>
auto get_rhs(assign_expression<Lhs, Operator, Rhs> e) -> Rhs {
  return read.rhs(e);
}

template <typename Lhs, typename Operator, typename Rhs>
auto get_rhs(dynamic_t<assign_expression<Lhs, Operator, Rhs>> e) -> dynamic_t<Rhs> {
  if (e.has_value()) {
    return {read.rhs(e.value())};
  }
  return {std::nullopt};
}

template <typename Lhs, typename Rhs>
constexpr bool are_correct_assignment_args =
    not is_const<Lhs>::value and values_are_assignable<Lhs, Rhs>::value and
    (can_be_null<Lhs>::value or not can_be_null<Rhs>::value);

template <typename Lhs>
constexpr bool are_correct_assignment_args<Lhs, default_value_t> =
    not is_const<Lhs>::value and has_default<Lhs>::value;

template <typename Lhs, typename Operator, typename Rhs>
struct is_assignment<assign_expression<Lhs, Operator, Rhs>>
    : public std::true_type {};

template <typename Lhs, typename Operator, typename Rhs>
struct nodes_of<assign_expression<Lhs, Operator, Rhs>> {
  using type = detail::type_vector<Lhs, Rhs>;
};

template <typename Lhs, typename Operator, typename Rhs>
struct lhs<assign_expression<Lhs, Operator, Rhs>> {
  using type = Lhs;
};

template <typename Lhs, typename Operator, typename Rhs>
struct rhs<assign_expression<Lhs, Operator, Rhs>> {
  using type = Rhs;
};

template <typename Context, typename Lhs, typename Operator, typename Rhs>
auto to_sql_string(Context& context, const assign_expression<Lhs, Operator, Rhs>& t)
    -> std::string {
  return to_sql_string(context, simple_column(read.lhs(t))) + Operator::symbol +
         operand_to_sql_string(context, read.rhs(t));
}

struct op_assign {
  static constexpr auto symbol = " = ";
};

template <typename _Table, typename ColumnSpec, typename Rhs>
  requires(are_correct_assignment_args<column_t<_Table, ColumnSpec>, Rhs>)
constexpr auto assign(column_t<_Table, ColumnSpec> column, Rhs value)
    -> assign_expression<column_t<_Table, ColumnSpec>, op_assign, Rhs> {
  return {std::move(column), std::move(value)};
}

}  // namespace sqlpp
