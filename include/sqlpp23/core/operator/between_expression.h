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
template <typename Lhs, typename Low, typename High>
struct between_expression : public enable_as, public enable_comparison {
  constexpr between_expression(Lhs lhs, Low low, High high)
      : _lhs(std::move(lhs)), _low(std::move(low)), _high(std::move(high)) {}
  between_expression(const between_expression&) = default;
  between_expression(between_expression&&) = default;
  between_expression& operator=(const between_expression&) = default;
  between_expression& operator=(between_expression&&) = default;
  ~between_expression() = default;

  Lhs _lhs;
  Low _low;
  High _high;
};

template <typename Lhs, typename Low, typename High>
struct data_type_of<between_expression<Lhs, Low, High>>
    : public std::conditional<
          sqlpp::is_optional<data_type_of_t<Lhs>>::value or
              sqlpp::is_optional<data_type_of_t<Low>>::value or
              sqlpp::is_optional<data_type_of_t<High>>::value,
          std::optional<boolean>,
          boolean> {};

template <typename Lhs, typename Low, typename High>
struct nodes_of<between_expression<Lhs, Low, High>> {
  using type = detail::type_vector<Lhs, Low, High>;
};

template <typename Lhs, typename Low, typename High>
struct requires_parentheses<between_expression<Lhs, Low, High>>
    : public std::true_type {};

template <typename Context, typename Lhs, typename Low, typename High>
auto to_sql_string(Context& context, const between_expression<Lhs, Low, High>& t)
    -> std::string {
  // Note: Temporary required to enforce parameter ordering.
  auto ret_val = operand_to_sql_string(context, read.lhs(t)) + " BETWEEN ";
  ret_val += operand_to_sql_string(context, read.low(t)) + " AND ";
  return ret_val + operand_to_sql_string(context, read.high(t));
}

}  // namespace sqlpp
