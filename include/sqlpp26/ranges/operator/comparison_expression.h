/*
 * Copyright (c) 2026, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sqlpp26/core/operator/comparison_expression.h>

namespace sqlpp::filter {

template <typename Lhs, typename Operator, typename Rhs>
struct comparison_expression{
template <typename Struct>
  constexpr auto operator()(const Struct& t) const {
    return _operator(_lhs(t), _rhs(t));
  }

  Lhs _lhs;
  Operator _operator;
  Rhs _rhs;
};

template<typename Operator>
struct filter_operator_of;

template<typename Operator>
using filter_operator_of_t = typename filter_operator_of<Operator>::type;

template<> struct filter_operator_of<::sqlpp::less> { using type = std::less<void>; };
template<> struct filter_operator_of<::sqlpp::less_equal> { using type = std::less_equal<void>; };
template<> struct filter_operator_of<::sqlpp::equal_to> { using type = std::equal_to<void>; };
template<> struct filter_operator_of<::sqlpp::not_equal_to> { using type = std::not_equal_to<void>; };
template<> struct filter_operator_of<::sqlpp::greater_equal> { using type = std::greater_equal<void>; };
template<> struct filter_operator_of<::sqlpp::greater> { using type = std::greater<void>; };

}

namespace sqlpp {

template <typename Lhs, typename Operator, typename Rhs>
constexpr auto to_filter_expression(const comparison_expression<Lhs, Operator, Rhs>& expr) {
  return filter::comparison_expression{to_filter_expression(read.lhs(expr)), filter::filter_operator_of_t<Operator>{},
                               to_filter_expression(read.rhs(expr))};
}

}  // namespace sqlpp

