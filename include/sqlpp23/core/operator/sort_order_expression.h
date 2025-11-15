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

#include <type_traits>

#include <sqlpp23/core/operator/comparison_functions.h>
#include <sqlpp23/core/reader.h>

namespace sqlpp {
template <typename L>
struct sort_order_expression {
  constexpr sort_order_expression(L l, sort_type r)
      : _lhs(std::move(l)), _rhs(std::move(r)) {}
  sort_order_expression(const sort_order_expression&) = default;
  sort_order_expression(sort_order_expression&&) = default;
  sort_order_expression& operator=(const sort_order_expression&) = default;
  sort_order_expression& operator=(sort_order_expression&&) = default;
  ~sort_order_expression() = default;

 private:
  friend reader_t;
  L _lhs;
  sort_type _rhs;
};

template <typename L>
struct nodes_of<sort_order_expression<L>> {
  using type = detail::type_vector<L>;
};

template <typename L>
struct is_sort_order<sort_order_expression<L>> : std::true_type {};

template <typename Context>
auto to_sql_string(Context&, const sort_type& t) -> std::string {
  if (t == sort_type::asc) {
    return " ASC";
  }
  return " DESC";
}

template <typename Context, typename L>
auto to_sql_string(Context& context, const sort_order_expression<L>& t)
    -> std::string {
  return operand_to_sql_string(context, read.lhs(t)) + to_sql_string(context, read.rhs(t));
}

}  // namespace sqlpp
