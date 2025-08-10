#pragma once

/*
 * Copyright (c) 2013-2015, Roland Bock
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

#include <sqlpp23/core/basic/value.h>
#include <sqlpp23/core/database/parameter_list.h>
#include <sqlpp23/core/database/prepared_select.h>
#include <sqlpp23/core/detail/get_first.h>
#include <sqlpp23/core/detail/get_last.h>
#include <sqlpp23/core/detail/pick_arg.h>
#include <sqlpp23/core/hidden.h>
#include <sqlpp23/core/noop.h>
#include <sqlpp23/core/query/statement_constructor_arg.h>
#include <sqlpp23/core/query/statement_fwd.h>
#include <sqlpp23/core/result.h>
#include <sqlpp23/core/result_type_provider.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/core/wrapped_static_assert.h>
#include <type_traits>
#include <sqlpp23/core/detail/type_vector.h>

namespace sqlpp {
class assert_no_unknown_ctes_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>,
                        "one clause requires common table expressions "
                        "which are otherwise not known in the statement");
  }
};

class assert_no_unknown_static_ctes_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>,
                        "one clause statically requires common table "
                        "expressions which are only "
                        "known dynamically in the statement");
  }
};

class assert_no_unknown_tables_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>,
                        "one clause requires tables which are otherwise "
                        "not known in the statement");
  }
};

class assert_no_unknown_static_tables_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>,
                        "one clause statically requires tables which are "
                        "only known dynamically in the statement");
  }
};

class assert_no_duplicate_table_providers_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(
        wrong<T...>,
        "at least one table is provided by two clauses, e.g. FROM and USING");
  }
};

class assert_no_parameters_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>,
                  "cannot execute statements with parameters directly, use "
                  "prepare instead");
  }
};

template <typename... Clauses>
using result_methods_t =
    result_methods_of_t<result_type_provider_t<Clauses...>>;

template <typename... Clauses>
struct statement_t : public Clauses..., public result_methods_t<Clauses...> {
  // Calculate provided/required CTEs and tables across all clauses
  using _all_provided_tables =
      detail::make_joined_set_t<provided_tables_of_t<Clauses>...>;
  using _all_provided_static_tables =
      detail::make_joined_set_t<provided_static_tables_of_t<Clauses>...>;

  using _all_required_tables =
      detail::make_joined_set_t<required_tables_of_t<Clauses>...>;
  using _all_required_static_tables =
      detail::make_joined_set_t<required_static_tables_of_t<Clauses>...>;

  using _all_provided_ctes =
      detail::make_joined_set_t<provided_ctes_of_t<Clauses>...>;
  using _all_provided_static_ctes =
      detail::make_joined_set_t<provided_static_ctes_of_t<Clauses>...>;

  using _all_required_ctes =
      detail::make_joined_set_t<required_ctes_of_t<Clauses>...>;
  using _all_required_static_ctes =
      detail::make_joined_set_t<required_static_ctes_of_t<Clauses>...>;

  using _all_provided_static_aggregates = detail::make_joined_set_t<
      known_static_aggregate_columns_of_t<Clauses>...>;
  using _all_provided_aggregates =
      detail::make_joined_set_t<known_aggregate_columns_of_t<Clauses>...>;

  // Calculate the unknown (i.e. required but not provided) tables and CTEs
  using _unknown_required_tables_of =
      detail::make_difference_set_t<_all_required_tables, _all_provided_tables>;
  using _unknown_required_static_tables_of =
      detail::make_difference_set_t<_all_required_static_tables,
                                    _all_provided_static_tables>;
  using _unknown_required_ctes_of =
      detail::make_difference_set_t<_all_required_ctes, _all_provided_ctes>;
  using _unknown_required_static_ctes_of =
      detail::make_difference_set_t<_all_required_static_ctes,
                                    _all_provided_static_ctes>;

  template <typename Expression>
  static constexpr bool _no_unknown_tables =
      _all_provided_tables::contains_all(required_tables_of_t<Expression>{});

  template <typename Expression>
  static constexpr bool _no_unknown_static_tables =
      _all_provided_static_tables::contains_all(
          required_static_tables_of_t<Expression>{});

  using _result_type_provider =
      detail::get_last_if_t<is_result_clause, noop, Clauses...>;

  using _table_check = static_combined_check_t<
      static_check_t<_unknown_required_tables_of::empty(),
                     assert_no_unknown_tables_t>,
      static_check_t<_unknown_required_static_tables_of::empty(),
                     assert_no_unknown_static_tables_t>,
      static_check_t<
          detail::are_disjoint<provided_tables_of_t<Clauses>...>::value,
          assert_no_duplicate_table_providers_t>>;
  using _cte_check = static_combined_check_t<
      static_check_t<_unknown_required_ctes_of::empty(),
                     assert_no_unknown_ctes_t>,
      static_check_t<_unknown_required_static_ctes_of::empty(),
                     assert_no_unknown_static_ctes_t>>;

  using _parameters = detail::type_vector_cat_t<parameters_of_t<Clauses>...>;

  using _parameter_check =
      static_check_t<_parameters::empty(), assert_no_parameters_t>;

  // Constructors
  statement_t() = default;

  template <typename... Fragments>
  statement_t(statement_constructor_arg<Fragments...> arg) : Clauses{arg}... {}

  statement_t(const statement_t& r) = default;
  statement_t(statement_t&& r) = default;
  statement_t& operator=(const statement_t& r) = default;
  statement_t& operator=(statement_t&& r) = default;
  ~statement_t() = default;
};




template <typename... Clauses>
struct can_be_used_as_table<statement_t<Clauses...>> {
  // A select can be used as a pseudo table if
  //   - at least one column is selected
  //   - the select is complete (leaks no table requirements or cte
  //   requirements)
  using _S = statement_t<Clauses...>;
  static constexpr bool value = has_result_row<_S>::value and
                                _S::_unknown_required_tables_of::empty() and
                                _S::_unknown_required_ctes_of::empty();
};

template <typename... Clauses>
struct no_of_result_columns<statement_t<Clauses...>>
    : public no_of_result_columns<result_type_provider_t<Clauses...>> {};

template <typename... Clauses>
struct result_methods_of<statement_t<Clauses...>> {
  using type = result_methods_t<Clauses...>;
};

template <typename... Clauses>
struct is_statement<statement_t<Clauses...>> : public std::true_type {};

template <typename... Clauses>
struct has_result_row<statement_t<Clauses...>>
    : public has_result_row<result_type_provider_t<Clauses...>> {};

template <typename... Clauses>
struct get_result_row<statement_t<Clauses...>> {
  using type = result_row_of_t<statement_t<Clauses...>,
                               result_type_provider_t<Clauses...>>;
};

template <typename... Clauses>
struct is_result_clause<statement_t<Clauses...>> {
  static constexpr bool value = not std::is_same<
      noop,
      typename statement_t<Clauses...>::_result_type_provider>::value;
};

template <typename... Clauses>
struct data_type_of<statement_t<Clauses...>> {
  using type = std::conditional_t<
      statement_consistency_check_t<statement_t<Clauses...>>::value,
      data_type_of_t<result_type_provider_t<Clauses...>>,
      no_value_t>;
};

// statements explicitly do not expose any nodes as most recursive traits
// should not traverse into sub queries, e.g.
//   - contains_aggregates
//   - known_aggregate_columns_of
template <typename... Clauses>
struct nodes_of<statement_t<Clauses...>> : public no_nodes {
};

template <typename Context, typename... Clauses>
struct compatibility_check<Context, statement_t<Clauses...>> {
  using type = compatibility_check_t<Context, detail::type_vector<Clauses...>>;
};

template <typename... Clauses>
struct required_insert_columns_of<statement_t<Clauses...>> {
  using type =
      detail::make_joined_set_t<required_insert_columns_of_t<Clauses>...>;
};

template <typename... Clauses>
struct parameters_of<statement_t<Clauses...>> {
  using type = detail::type_vector_cat_t<parameters_of_t<Clauses>...>;
};

template <typename... Clauses>
struct provided_optional_tables_of<statement_t<Clauses...>> {
  using type =
      detail::make_joined_set_t<provided_optional_tables_of_t<Clauses>...>;
};

template <typename... Clauses>
struct required_tables_of<statement_t<Clauses...>> {
  using type = typename statement_t<Clauses...>::_unknown_required_tables_of;
};

template <typename... Clauses>
struct required_static_tables_of<statement_t<Clauses...>> {
  using type =
      typename statement_t<Clauses...>::_unknown_required_static_tables_of;
};

template <typename... Clauses>
struct required_ctes_of<statement_t<Clauses...>> {
  using type = typename statement_t<Clauses...>::_unknown_required_ctes_of;
};

template <typename... Clauses>
struct required_static_ctes_of<statement_t<Clauses...>> {
  using type =
      typename statement_t<Clauses...>::_unknown_required_static_ctes_of;
};

template <typename... Clauses>
struct requires_parentheses<statement_t<Clauses...>> : public std::true_type {};

template <typename... Clauses>
struct statement_consistency_check<statement_t<Clauses...>> {
  using type = static_combined_check_t<
      consistency_check_t<statement_t<Clauses...>, Clauses>...>;
};

template <typename... Clauses>
[[nodiscard]] auto check_table_consistency(const statement_t<Clauses...>&) {
  using S = statement_t<Clauses...>;
  return typename S::_table_check{} and typename S::_cte_check{};
}

template <typename... Clauses>
[[nodiscard]] auto check_parameter_consistency(const statement_t<Clauses...>&) {
  using S = statement_t<Clauses...>;
  return typename S::_parameter_check{};
}

template <typename... Clauses>
[[nodiscard]] constexpr auto check_basic_consistency(const statement_t<Clauses...>&) {
  return (consistent_t{} && ... &&
          consistency_check_t<statement_t<Clauses...>, Clauses>{});
};

template <typename... Clauses>
[[nodiscard]] constexpr auto check_prepare_consistency(const statement_t<Clauses...>& t) {
  return (check_basic_consistency(t) && ... &&
          prepare_check_t<statement_t<Clauses...>, Clauses>{})
    && (typename statement_t<Clauses...>::_table_check{})
    && (typename statement_t<Clauses...>::_cte_check{});
};

template <typename... Clauses>
[[nodiscard]] constexpr auto check_run_consistency(const statement_t<Clauses...>& t) {
  return (check_prepare_consistency(t) && ... &&
          run_check_t<statement_t<Clauses...>, Clauses>{})
    && (typename statement_t<Clauses...>::_table_check{})
    && (typename statement_t<Clauses...>::_parameter_check{});
};

template <typename... Clauses>
struct statement_prepare_check<statement_t<Clauses...>> {
  using type = static_combined_check_t<
      statement_consistency_check_t<statement_t<Clauses...>>,
      static_combined_check_t<
          prepare_check_t<statement_t<Clauses...>, Clauses>...>,
      typename statement_t<Clauses...>::_table_check,
      typename statement_t<Clauses...>::_cte_check>;
};

template <typename... Clauses>
struct statement_run_check<statement_t<Clauses...>> {
  using type = static_combined_check_t<
      statement_prepare_check_t<statement_t<Clauses...>>,
      static_combined_check_t<run_check_t<statement_t<Clauses...>, Clauses>...>,
      typename statement_t<Clauses...>::_parameter_check>;
};

template <typename OldClause, typename... Clauses, typename NewClause>
auto new_statement(statement_t<Clauses...> oldStatement, NewClause newClause)
    -> statement_t<std::conditional_t<std::is_same<Clauses, OldClause>::value,
                                      NewClause,
                                      Clauses>...> {
  return statement_t<std::conditional_t<std::is_same<Clauses, OldClause>::value,
                                        NewClause, Clauses>...>{
      statement_constructor_arg(oldStatement, newClause)};
}

template <typename T>
struct core_statement;
template <typename... Clauses>
struct core_statement<detail::type_vector<Clauses...>> {
  using type = statement_t<Clauses...>;
};

template <typename... Clauses>
using core_statement_t = typename core_statement<
    detail::copy_if_t<detail::type_vector<Clauses...>, is_clause>>::type;

template <typename Statement>
struct statement_has_unique_clauses;

template <typename... Clauses>
struct statement_has_unique_clauses<statement_t<Clauses...>>
    : public detail::are_unique<Clauses...> {};

template <typename... LClauses, typename... RClauses>
constexpr auto operator<<(statement_t<LClauses...> l,
                          statement_t<RClauses...> r)
    -> core_statement_t<LClauses..., RClauses...> {
  using _core_statement = core_statement_t<LClauses..., RClauses...>;
  static_assert(statement_has_unique_clauses<_core_statement>::value,
                      "statements must contain unique clauses only");
  return _core_statement(statement_constructor_arg(std::move(l), std::move(r)));
}

template <typename... LClauses, typename Clause>
constexpr auto operator<<(statement_t<LClauses...> l, Clause r)
    -> core_statement_t<LClauses..., Clause> {
  static_assert(
      is_clause<Clause>::value,
      "statement_t::operator<< requires statements or clauses as parameters");
  using _core_statement = core_statement_t<LClauses..., Clause>;
  static_assert(statement_has_unique_clauses<_core_statement>::value,
                      "statements must contain unique clauses only");
  return _core_statement(statement_constructor_arg(std::move(l), std::move(r)));
}

template <typename Context, typename... Clauses>
auto to_sql_string(Context& context, const statement_t<Clauses...>& t)
    -> std::string {
  check_compatibility<Context>(t).verify();
  auto result = std::string{};
  ((result += to_sql_string(context, static_cast<const Clauses&>(t))), ...);

  return result;
}

}  // namespace sqlpp
