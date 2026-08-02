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
#include <sqlpp26/core/aggregate_function/count.h>
#include <sqlpp26/core/indices.h>
#include <sqlpp26/ranges/type_traits.h>

namespace sqlpp::ranges {

template <typename Expression>
struct count {
  constexpr auto aggregate(const auto& range) const {
    return std::ranges::fold_left(range, int64_t{},
                                  [](const auto& result, const auto& ) {
                                    return result + 1;
                                  });
  }

  Expression _expression;
};

template<typename Expression>
struct is_aggregate_function<count<Expression>> : public std::true_type {};

}  // namespace sqlpp::ranges

namespace sqlpp {
  //TODO: Handle Flag
template <typename Flag, typename Expression>
constexpr auto to_filter_expression(const count_t<Flag, Expression>& t) {
  return ranges::count{to_filter_expression(read.expression(t))};
}
}  // namespace sqlpp
