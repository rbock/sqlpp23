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

#include <ranges>

#include <sqlpp26/ranges/type_traits.h>
#include <sqlpp26/ranges/to_filter_expression.h>
#include <sqlpp26/core/clause/select_column_list.h>
#include <sqlpp26/core/indices.h>

namespace sqlpp::ranges {

template <typename Struct, typename Accessor>
struct result_member_type {
          using type = std::decay_t<decltype(std::declval<Accessor>()(
              std::declval<Struct>()))>;
};
template <typename Struct, typename Accessor>
requires(is_aggregate_function_v<Accessor>)
struct result_member_type<Struct, Accessor> {
  using type = std::decay_t<decltype(std::declval<Accessor>().aggregate(
      std::declval<std::vector<Struct>>()))>;
};

template <typename Struct, typename... Accessors>
struct result_row {
  struct type;
  consteval {
    std::vector<std::meta::info> row_data_members;
    template for (constexpr auto index : std::views::iota(size_t{}, sizeof...(Accessors))) {
      using T = typename result_member_type<Struct, Accessors...[index]>::type;
      row_data_members.push_back(std::meta::data_member_spec(
          ^^T,
          {.name = ::sqlpp::name_of_v<Accessors...[index]>}));
    }
    define_aggregate(^^type, row_data_members);
  }
};
template <typename Struct, typename... Accessors>
using result_row_t = typename result_row<Struct, Accessors...>::type;

template <typename... Accessors>
struct select_column_list {
  template <typename Struct>
  constexpr auto select_from_row(const Struct& s) const {
    static constexpr auto [... Idx] = indices<sizeof...(Accessors)>;
    return result_row_t<Struct, Accessors...>{std::get<Idx>(_accessors)(s)...};
  }

  template <std::size_t Idx, typename Range>
  constexpr auto fold(const Range& r) const {
    if constexpr (is_aggregate_function_v<Accessors...[Idx]>) {
      return std::get<Idx>(_accessors).aggregate(r);
    }
    else {
      return std::get<Idx>(_accessors)(r.front());
    }
  }

  template <typename Range>
  constexpr auto select_from_chunk(const Range& r) const {
    static constexpr auto [... Idx] = indices<sizeof...(Accessors)>;
    using Struct = std::decay_t<decltype(r.front())>;
    return result_row_t<Struct, Accessors...>{fold<Idx>(r)...};
  }

  std::tuple<Accessors...> _accessors;
};

}  // namespace sqlpp::ranges

namespace sqlpp {
template <typename... Flags, typename... Columns>
constexpr auto to_filter_expression(
    const select_column_list_t<std::tuple<Flags...>, std::tuple<Columns...>>& t) {
  static constexpr auto [...Idx] = indices<sizeof...(Columns)>;
  return ranges::select_column_list{std::make_tuple(to_filter_expression(std::get<Idx>(read.columns(t)))...)};
}
}  // namespace sqlpp::ranges
