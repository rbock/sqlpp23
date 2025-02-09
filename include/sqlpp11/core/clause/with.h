#pragma once

/*
 * Copyright (c) 2013-2016, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sqlpp11/core/operator/assign_expression.h>
#include <sqlpp11/core/basic/column_fwd.h>
#include <sqlpp11/core/tuple_to_sql_string.h>
#include <sqlpp11/core/logic.h>
#include <sqlpp11/core/no_data.h>
#include <sqlpp11/core/query/statement_fwd.h>
#include <sqlpp11/core/type_traits.h>

#include <sqlpp11/core/clause/clause_base.h>
#include <sqlpp11/core/clause/cte.h>

namespace sqlpp
{
  template <typename... Ctes>
  struct with_t
  {
    with_t(Ctes... expressions) : _expressions(expressions...)
    {
    }

    with_t(const with_t&) = default;
    with_t(with_t&&) = default;
    with_t& operator=(const with_t&) = default;
    with_t& operator=(with_t&&) = default;
    ~with_t() = default;

    std::tuple<Ctes...> _expressions;
  };

  template <typename... Ctes>
  struct is_clause<with_t<Ctes...>> : public std::true_type
  {
  };

  template <typename Statement, typename... Ctes>
  struct consistency_check<Statement, with_t<Ctes...>>
  {
      // FIXME: Need real checks here
    using type = consistent_t;
  };

  // Note: No nodes are exposed directly. Nothing should be leaked from CTEs by accident.

  template <typename... Ctes>
  struct provided_ctes_of<with_t<Ctes...>>
  {
    using type = detail::make_joined_set_t<provided_ctes_of_t<Ctes>...>;
  };

  template <typename... Ctes>
  struct provided_static_ctes_of<with_t<Ctes...>>
  {
    using type = detail::make_joined_set_t<provided_static_ctes_of_t<Ctes>...>;
  };

  template <typename... Ctes>
  struct parameters_of<with_t<Ctes...>>
  {
    using type = detail::type_vector_cat_t<parameters_of_t<Ctes>...>;
  };

  struct no_with_t
  {
  };

  template <typename Statement>
  struct clause_base<no_with_t, Statement> : public clause_data<no_with_t, Statement>
  {
    using clause_data<no_with_t, Statement>::clause_data;

    template <typename... Ctes>
    auto with(with_t<Ctes...> with) const
        -> decltype(new_statement(*this, with))
    {
      return new_statement(*this, with);
    }
  };

  template <typename Statement>
  struct consistency_check<Statement, no_with_t>
  {
    using type = consistent_t;
  };

  template <typename... Ctes>
  struct blank_with_t
  {
    with_t<Ctes...> _with_clause;

    template <typename Statement>
    auto operator()(Statement statement)
        -> decltype(statement.with(_with_clause))
    {
      return statement.with(_with_clause);
    }
  };

  // Interpreters
  template <typename Context>
  auto to_sql_string(Context&, const no_with_t&) -> std::string
  {
    return "";
  }

  template <typename Context, typename... Ctes>
  auto to_sql_string(Context& context, const with_t<Ctes...>& t) -> std::string
  {
    static constexpr bool _is_recursive = logic::any<is_recursive_cte<Ctes>::value...>::value;

    return std::string("WITH ") + (_is_recursive ? "RECURSIVE " : "") +
           tuple_to_sql_string(context, t._expressions, tuple_operand{", "}) + " ";
  }

  template <typename Context, typename... Ctes>
  auto to_sql_string(Context& context, const blank_with_t<Ctes...>& t) -> std::string
  {
    return to_sql_string(context, t._with_clause);
  }

  // CTEs can depend on CTEs defined before (in the same query).
  // `have_correct_cte_dependencies` checks that by walking the CTEs from left to right and building a type vector that
  // contains the CTE it already has looked at.
  template <typename AllowedCTEs, typename... CTEs>
    struct have_correct_cte_dependencies_impl;

  template <typename AllowedCTEs>
    struct have_correct_cte_dependencies_impl<AllowedCTEs>: public std::true_type {};

  template <typename AllowedCTEs, typename CTE, typename... Rest>
    struct have_correct_cte_dependencies_impl<AllowedCTEs, CTE, Rest...>
    {
      using allowed_ctes = detail::make_joined_set_t<AllowedCTEs, provided_ctes_of_t<CTE>>;
      static constexpr bool value =
          allowed_ctes::contains_all(required_ctes_of_t<CTE>{}) and
          have_correct_cte_dependencies_impl<allowed_ctes, Rest...>::value;
    };

  template <typename... CTEs>
  struct have_correct_cte_dependencies
  {
    static constexpr bool value = have_correct_cte_dependencies_impl<detail::type_set<>, detail::type_set<>, CTEs...>::value;
  };

  template <typename AllowedStaticCTEs, typename... CTEs>
    struct have_correct_static_cte_dependencies_impl;

  template <typename AllowedStaticCTEs>
    struct have_correct_static_cte_dependencies_impl<AllowedStaticCTEs>: public std::true_type {};

  template <typename AllowedStaticCTEs, typename CTE, typename... Rest>
    struct have_correct_static_cte_dependencies_impl<AllowedStaticCTEs, CTE, Rest...>
    {
      using allowed_static_ctes = detail::make_joined_set_t<AllowedStaticCTEs, provided_static_ctes_of_t<CTE>>;
      static constexpr bool value =
          allowed_static_ctes::contains_all(required_static_ctes_of_t<CTE>{}) and
          have_correct_static_cte_dependencies_impl<allowed_static_ctes, Rest...>::value;
    };

  template <typename... CTEs>
  struct have_correct_static_cte_dependencies
  {
    static constexpr bool value = have_correct_static_cte_dependencies_impl<detail::type_set<>, detail::type_set<>, CTEs...>::value;
  };

  template <typename... Ctes, typename = std::enable_if_t<logic::all<is_cte<remove_dynamic_t<Ctes>>::value...>::value>>
  auto with(Ctes... cte) -> blank_with_t<Ctes...>
  {
    SQLPP_STATIC_ASSERT(have_correct_cte_dependencies<Ctes...>::value,
                        "at least one CTE depends on another CTE that is not defined left of it");
    SQLPP_STATIC_ASSERT(
        have_correct_static_cte_dependencies<Ctes...>::value,
        "at least one CTE statically depends on another CTE that is not defined statically left of it (only dynamically)");
    SQLPP_STATIC_ASSERT(detail::are_unique<make_char_sequence_t<Ctes>...>::value,
                        "CTEs in with need to have unique names");

    return {{cte...}};
  }
}  // namespace sqlpp
