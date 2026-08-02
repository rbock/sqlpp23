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
#include <sqlpp26/core/clause/group_by.h>
#include <sqlpp26/core/indices.h>

namespace sqlpp::ranges {

struct no_group_by {
};

template <typename... Accessors>
struct group_by {
  template <typename Struct>
  constexpr auto operator()(const Struct& s) const {
    static constexpr auto [... Idx] = indices<sizeof...(Accessors)>;
    return std::tie(std::get<Idx>(_accessors)(s)...);
  }

  std::tuple<Accessors...> _accessors;
};

}  // namespace sqlpp::ranges

namespace sqlpp {
constexpr auto to_filter_expression(const no_group_by_t&) {
  return ranges::no_group_by{};
}
template <typename... Flags, typename... Expressions>
constexpr auto to_filter_expression(
    const group_by_t<Expressions...>& t) {
  static constexpr auto [...Idx] = indices<sizeof...(Expressions)>;
  return ranges::group_by{std::make_tuple(to_filter_expression(std::get<Idx>(read.expressions(t)))...)};
}
}  // namespace sqlpp::ranges
