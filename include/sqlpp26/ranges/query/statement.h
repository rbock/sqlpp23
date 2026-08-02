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
#include <cstddef>
#include <cstdlib>
#include <ranges>


namespace sqlpp::ranges {
template <typename...>
struct group_by;

template <typename...>
struct insert_assignments;

template <typename...>
struct select_column_list;

template <typename...>
struct update_assignments;

template <typename>
struct where;

template<std::meta::info Template, typename... Clauses>
consteval auto clause_index_of() -> std::optional<std::size_t> {
  const auto clauses = std::vector{^^Clauses...};
  if (const auto it = std::ranges::find_if(clauses, [](const auto& clause) {
        return has_template_arguments(clause) and
               template_of(clause) == Template;
      }); it != clauses.end()) {
    return std::distance(clauses.begin(), it);
  }
  return {};
}

template<std::meta::info Template, typename... Clauses>
consteval auto required_clause_index_of() -> std::size_t {
  if (const auto index = clause_index_of<Template, Clauses...>()) {
    return *index;
  }
  throw std::domain_error(std::string("Could not find required clause: ") + identifier_of(Template));
}

template <typename... Clauses>
struct statement {
  template <std::meta::info Template>
  static constexpr auto get_clause_index() -> std::optional<std::size_t> {
    return clause_index_of<Template, Clauses...>();
  }

  template <std::meta::info Template>
  constexpr const auto& get_clause() const {
    return std::get<required_clause_index_of<Template, Clauses...>()>(_filter_clauses);
  }

  template <typename Struct>
  constexpr auto insert(std::vector<Struct>& v) const {
    get_clause<^^insert_assignments>()(v);

    return v.back();
  }

  template <typename Struct>
  constexpr auto update(std::vector<Struct>& t) const {
    auto filter = std::views::filter([this](const auto& row) {
      if constexpr (get_clause_index<^^where>()) {
        return get_clause<^^where>()(row);
      }
      return true;
    });
    auto update = std::views::transform([this](auto& row) -> auto& {
      get_clause<^^update_assignments>()(row);
      return row;
    });
    for (auto&& _ : t | filter | update) {
    }
  }

  template <typename Struct>
    requires(not get_clause_index<^^group_by>().has_value())
  constexpr auto select(const std::vector<Struct>& t) const {
    const auto filter = std::views::filter([this](const auto& row) {
      if constexpr (get_clause_index<^^where>()) {
        return get_clause<^^where>()(row);
      }
      return true;
    });

    const auto select = std::views::transform([this](const auto& row) -> auto {
      return get_clause<^^select_column_list>().select_from_row(row);
    });

    return t | filter | select;
  }

  template <typename Struct>
    requires(get_clause_index<^^group_by>().has_value())
  constexpr auto select(const std::vector<Struct>& t) const {
    // Apply WHERE
    const auto filter = std::views::filter([this](const auto& row) {
      if constexpr (get_clause_index<^^where>()) {
        return get_clause<^^where>()(row);
      }
      return true;
    });
    std::vector filtered = t | filter | std::ranges::to<std::vector>();

    // Apply GROUP BY
    // This requires sorting, which requires taking a copy since we don't want
    // to mess with the original.
    // After that, we can chunk into ranges of equal rows based on GROUP BY
    // expressions.
    std::ranges::sort(filtered, [this](const auto& a, const auto& b) {
        const auto clause = get_clause<^^group_by>();
      return clause(a) < clause(b);
    });

    auto chunks =
        filtered | std::views::chunk_by([this](const auto& a, const auto& b) {
          const auto clause = get_clause<^^group_by>();
          return clause(a) == clause(b);
        });

    // Transform each chunk
    const auto selector = [this](const auto& chunk) {
      return get_clause<^^select_column_list>().select_from_chunk(chunk);
    };
    auto result = chunks | std::views::transform(selector) |
                  std::ranges::to<std::vector>();

    return result;
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
