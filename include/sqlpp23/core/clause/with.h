#pragma once

/*
 * Copyright (c) 2013-2016, Roland Bock
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

#include <sqlpp23/core/basic/column_fwd.h>
#include <sqlpp23/core/clause/cte.h>
#include <sqlpp23/core/logic.h>
#include <sqlpp23/core/no_data.h>
#include <sqlpp23/core/operator/assign_expression.h>
#include <sqlpp23/core/query/statement.h>
#include <sqlpp23/core/reader.h>
#include <sqlpp23/core/tuple_to_sql_string.h>
#include <sqlpp23/core/detail/type_vector.h>

namespace sqlpp {
struct no_with_t;

template <typename... Ctes>
struct with_t {
  with_t(std::tuple<Ctes...> ctes) : _ctes(std::move(ctes)) {}
  with_t(const with_t&) = default;
  with_t(with_t&&) = default;
  with_t& operator=(const with_t&) = default;
  with_t& operator=(with_t&&) = default;
  ~with_t() = default;

 private:
  friend reader_t;
  std::tuple<Ctes...> _ctes;
};

template <typename Context, typename... Ctes>
auto to_sql_string(Context& context, const with_t<Ctes...>& t) -> std::string {
  static constexpr bool _is_recursive =
      logic::any<is_recursive_cte<Ctes>::value...>::value;

  return std::string("WITH ") + (_is_recursive ? "RECURSIVE " : "") +
         tuple_to_sql_string(context, read.ctes(t), tuple_operand{", "}) + " ";
}

template <typename... Ctes>
struct is_clause<with_t<Ctes...>> : public std::true_type {};

template <typename Statement, typename... Ctes>
struct consistency_check<Statement, with_t<Ctes...>> {
  // FIXME: Need real checks here
  using type = consistent_t;
  constexpr auto operator()() {
    return type{};
  }
};

// Note: No nodes are exposed directly. Nothing should be leaked from CTEs by
// accident.

template <typename... Ctes>
struct nodes_of<with_t<Ctes...>> : public no_nodes {};

template <typename... Ctes>
struct provided_ctes_of<with_t<Ctes...>> {
  using type = detail::make_joined_set_t<provided_ctes_of_t<Ctes>...>;
};

template <typename... Ctes>
struct provided_static_ctes_of<with_t<Ctes...>> {
  using type = detail::make_joined_set_t<provided_static_ctes_of_t<Ctes>...>;
};

template <typename... Ctes>
struct parameters_of<with_t<Ctes...>> {
  using type = detail::type_vector_cat_t<parameters_of_t<Ctes>...>;
};

template <typename Context, typename... Ctes>
struct compatibility_check<Context, with_t<Ctes...>>
    : public compatibility_check<Context, detail::type_vector<Ctes...>> {};

// CTEs can depend on CTEs defined before (in the same query).
// `have_correct_cte_dependencies` checks that by walking the CTEs from left to
// right and building a type vector that contains the CTE it already has looked
// at.
template <typename AllowedCTEs, typename... CTEs>
struct have_correct_cte_dependencies_impl;

template <typename AllowedCTEs>
struct have_correct_cte_dependencies_impl<AllowedCTEs> : public std::true_type {
};

template <typename AllowedCTEs, typename CTE, typename... Rest>
struct have_correct_cte_dependencies_impl<AllowedCTEs, CTE, Rest...> {
  using allowed_ctes =
      detail::make_joined_set_t<AllowedCTEs, provided_ctes_of_t<CTE>>;
  static constexpr bool value =
      allowed_ctes::contains_all(required_ctes_of_t<CTE>{}) and
      have_correct_cte_dependencies_impl<allowed_ctes, Rest...>::value;
};

template <typename... CTEs>
struct have_correct_cte_dependencies {
  static constexpr bool value =
      have_correct_cte_dependencies_impl<detail::type_set<>,
                                         detail::type_set<>,
                                         CTEs...>::value;
};

template <typename AllowedStaticCTEs, typename... CTEs>
struct have_correct_static_cte_dependencies_impl;

template <typename AllowedStaticCTEs>
struct have_correct_static_cte_dependencies_impl<AllowedStaticCTEs>
    : public std::true_type {};

template <typename AllowedStaticCTEs, typename CTE, typename... Rest>
struct have_correct_static_cte_dependencies_impl<AllowedStaticCTEs,
                                                 CTE,
                                                 Rest...> {
  using allowed_static_ctes =
      detail::make_joined_set_t<AllowedStaticCTEs,
                                provided_static_ctes_of_t<CTE>>;
  static constexpr bool value =
      allowed_static_ctes::contains_all(required_static_ctes_of_t<CTE>{}) and
      have_correct_static_cte_dependencies_impl<allowed_static_ctes,
                                                Rest...>::value;
};

template <typename... CTEs>
struct have_correct_static_cte_dependencies {
  static constexpr bool value =
      have_correct_static_cte_dependencies_impl<detail::type_set<>,
                                                detail::type_set<>,
                                                CTEs...>::value;
};

struct no_with_t {
  template <typename Statement, DynamicCte... Ctes>
    requires(have_correct_cte_dependencies<Ctes...>::value and
             have_correct_static_cte_dependencies<Ctes...>::value and
             detail::are_unique<
                 make_char_sequence_t<remove_dynamic_t<Ctes>>...>::value)
  auto with(this Statement&& self, Ctes... ctes) {
    return new_statement<no_with_t>(
        std::forward<Statement>(self),
        with_t<Ctes...>{std::make_tuple(std::move(ctes)...)});
  }
};

template <typename Context>
auto to_sql_string(Context&, const no_with_t&) -> std::string {
  return "";
}

template <typename Statement>
struct consistency_check<Statement, no_with_t> {
  using type = consistent_t;
  constexpr auto operator()() {
    return type{};
  }
};

template <DynamicCte... Ctes>
    requires(have_correct_cte_dependencies<Ctes...>::value and
             have_correct_static_cte_dependencies<Ctes...>::value and
             detail::are_unique<
                 make_char_sequence_t<remove_dynamic_t<Ctes>>...>::value)
auto with(Ctes... ctes) {
  return statement_t<no_with_t>{}.with(std::move(ctes)...);
}
}  // namespace sqlpp
