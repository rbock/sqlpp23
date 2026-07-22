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

#include <sqlpp26/core/operator/logical_expression.h>

namespace sqlpp::filter {

template <typename Lhs, typename Operator, typename Rhs>
struct logical_expression{
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

template<> struct filter_operator_of<::sqlpp::logical_and> { using type = std::logical_and<void>; };
template<> struct filter_operator_of<::sqlpp::logical_or> { using type = std::logical_or<void>; };

// TODO logical_not

}

namespace sqlpp {

template <typename Operator, typename Lhs, typename Rhs>
constexpr auto to_filter_expression(const logical_expression<Operator, Lhs, Rhs>& expr) {
    // TODO: What to do with multiple elements?
  return filter::logical_expression{to_filter_expression(std::get<0>(read.expressions(expr))), filter::filter_operator_of_t<Operator>{},
                               to_filter_expression(std::get<1>(read.expressions(expr)))};
}

}  // namespace sqlpp

