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
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
template <typename L, typename R1, typename R2>
struct between_expression : public enable_as, public enable_comparison {
  constexpr between_expression(L l, R1 r1, R2 r2)
      : _l(std::move(l)), _r1(std::move(r1)), _r2(std::move(r2)) {}
  between_expression(const between_expression&) = default;
  between_expression(between_expression&&) = default;
  between_expression& operator=(const between_expression&) = default;
  between_expression& operator=(between_expression&&) = default;
  ~between_expression() = default;

  L _l;
  R1 _r1;
  R2 _r2;
};

template <typename L, typename R1, typename R2>
struct data_type_of<between_expression<L, R1, R2>>
    : public std::conditional<
          sqlpp::is_optional<data_type_of_t<L>>::value or
              sqlpp::is_optional<data_type_of_t<R1>>::value or
              sqlpp::is_optional<data_type_of_t<R2>>::value,
          std::optional<boolean>,
          boolean> {};

template <typename L, typename R1, typename R2>
struct nodes_of<between_expression<L, R1, R2>> {
  using type = detail::type_vector<L, R1, R2>;
};

template <typename L, typename R1, typename R2>
struct requires_parentheses<between_expression<L, R1, R2>>
    : public std::true_type {};

template <typename Context, typename L, typename R1, typename R2>
auto to_sql_string(Context& context, const between_expression<L, R1, R2>& t)
    -> std::string {
  // Note: Temporary required to enforce parameter ordering.
  auto ret_val = operand_to_sql_string(context, t._l) + " BETWEEN ";
  ret_val += operand_to_sql_string(context, t._r1) + " AND ";
  return ret_val + operand_to_sql_string(context, t._r2);
}

}  // namespace sqlpp
