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

#include <sqlpp26/core/query/statement.h>
#include <sqlpp26/core/indices.h>
#include <sqlpp26/ranges/type_traits.h>

namespace sqlpp::ranges {
template <typename Clause, typename Row>
constexpr auto filter_row(const Clause&, const Row&) -> bool { return true; }

template <typename Clause, typename Row>
  requires(is_filter_v<Clause>)
constexpr auto filter_row(const Clause& clause, const Row& r) -> bool { return clause(r); }

template <typename Clause, typename Range>
constexpr auto insert_row(const Clause&, Range&) -> void {}

template <typename Clause, typename Range>
  requires(is_inserter_v<Clause>)
constexpr auto insert_row(const Clause& clause, Range& r) -> void { clause(r); }

template <typename Clause, typename Row>
constexpr auto update_row(const Clause&, Row&) -> void {}

template <typename Clause, typename Row>
  requires(is_updater_v<Clause>)
constexpr auto update_row(const Clause& clause, Row& r) -> void { clause(r); }

template <typename... Clauses>
struct statement {
  template <typename Struct>
  constexpr auto insert(std::vector<Struct>& v) const {
    static constexpr auto [... Idx] = indices<sizeof...(Clauses)>;
    (..., insert_row(std::get<Idx>(_filter_clauses), v));

    return v.back();
  }

  template <typename Struct>
  constexpr auto update(std::vector<Struct>& t) const {
    static constexpr auto [... Idx] = indices<sizeof...(Clauses)>;

    auto filter = std::views::filter([this](const auto& row) { return (...&& filter_row(std::get<Idx>(_filter_clauses), row)); });
    auto transform = std::views::transform([this](auto& row) -> auto& {
        (..., update_row(std::get<Idx>(_filter_clauses), row));
        return row;
        });
    for (auto&& _ : t | filter | transform) {
    }
  }

  template <typename Struct>
  constexpr auto select(const std::vector<Struct>& t) const {
    static constexpr auto [... Idx] = indices<sizeof...(Clauses)>;

    auto filter = std::views::filter([this](const auto& row) { return (...&& filter_row(std::get<Idx>(_filter_clauses), row)); });
    auto transform = std::views::transform([this](const auto& row) -> auto {
        // TODO: Need to obtain the actual selec_column_list clause and apply it's logic
        return row;
        });
    return t | filter | transform;
  }

  std::tuple<Clauses...> _filter_clauses;
};
}  // namespace sqlpp::ranges

namespace sqlpp {
template <typename... Clauses>
constexpr auto to_filter_expression(
    const statement_t<Clauses...>& s) {
  return ranges::statement{std::make_tuple(to_filter_expression(static_cast<const Clauses&>(s))...)};
}
}  // namespace sqlpp::ranges
