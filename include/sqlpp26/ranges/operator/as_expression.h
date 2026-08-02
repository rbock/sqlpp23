#pragma once

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
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
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

#include <sqlpp26/ranges/to_filter_expression.h>
#include <sqlpp26/core/operator/as_expression.h>
#include <sqlpp26/core/indices.h>
#include <sqlpp26/ranges/type_traits.h>

namespace sqlpp::ranges {

template <typename Expression, fixed_string Name>
struct as_expression {
  constexpr auto operator()(const auto& row) const {
      return _filter(row);
  }

  constexpr auto aggregate(const auto& range) const {
      return _filter.aggregate(range);
  }

  Expression _filter;
};

template <typename Expression, fixed_string Name>
struct is_aggregate_function<as_expression<Expression, Name>> : public is_aggregate_function<Expression>{};

}  // namespace sqlpp::ranges

namespace sqlpp {
template <typename Expression, fixed_string Name>
struct name_of<ranges::as_expression<Expression, Name>> {
  static constexpr std::string_view value = name_of_v<as_expression<Expression, Name>>;
};

template <typename Expression, fixed_string Name>
constexpr auto to_filter_expression(const as_expression<Expression, Name>& t) {
  return ranges::as_expression<decltype(to_filter_expression(read.expression(t))), Name>{to_filter_expression(read.expression(t))};
}
}  // namespace sqlpp
