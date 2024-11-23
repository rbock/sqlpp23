#pragma once

/*
 * Copyright (c) 2013-2015, Roland Bock
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

#include <sqlpp11/core/noop.h>
#include <sqlpp11/core/database/parameter_list.h>
#include <sqlpp11/core/query/policy_update.h>
#include <sqlpp11/core/database/prepared_select.h>
#include <sqlpp11/core/result.h>
#include <sqlpp11/core/to_sql_string.h>
#include <sqlpp11/core/query/statement_fwd.h>

#include <sqlpp11/core/detail/get_first.h>
#include <sqlpp11/core/detail/get_last.h>
#include <sqlpp11/core/detail/pick_arg.h>

namespace sqlpp
{
  template <typename... Policies>
  struct statement_t;

  SQLPP_PORTABLE_STATIC_ASSERT(
      assert_no_unknown_ctes_t,
      "one clause requires common table expressions which are otherwise not known in the statement");
  SQLPP_PORTABLE_STATIC_ASSERT(assert_no_unknown_tables_t,
                               "one clause requires tables which are otherwise not known in the statement");
  SQLPP_PORTABLE_STATIC_ASSERT(assert_no_parameters_t,
                               "cannot run statements with parameters directly, use prepare instead");

  namespace detail
  {
    template <typename... Policies>
    constexpr auto is_any_policy_missing() -> bool
    {
      return logic::any<is_missing_t<Policies>::value...>::value;
    }

    template <typename... Policies>
    struct statement_policies_t
    {
      using _statement_t = statement_t<Policies...>;

      template <typename Needle, typename Replacement>
      struct _policies_update_t
      {
        static_assert(make_type_set_t<Policies...>::template count<Needle>(),
                      "policies update for non-policy class detected");
        using type = statement_t<policy_update_t<Policies, Needle, Replacement>...>;
      };

      template <typename Needle, typename Replacement>
      using _new_statement_t = typename _policies_update_t<Needle, Replacement>::type;

      using _all_required_ctes = detail::make_joined_set_t<required_ctes_of_t<Policies>...>;
      using _all_provided_ctes = detail::make_joined_set_t<provided_ctes_of_t<Policies>...>;
      using _all_required_tables = detail::make_joined_set_t<required_tables_of_t<Policies>...>;
      using _all_provided_tables = detail::make_joined_set_t<provided_tables_of_t<Policies>...>;
      using _all_provided_optional_tables = detail::make_joined_set_t<provided_optional_tables_of_t<Policies>...>;
#warning: provided_aggregates_of needs to be replaced with type_vector, too
      using _all_provided_aggregates = detail::make_joined_set_t<provided_aggregates_of<Policies>...>;

      using _required_tables_of = detail::make_difference_set_t<_all_required_tables, _all_provided_tables>;
      using _required_ctes_of = detail::make_difference_set_t<_all_required_ctes, _all_provided_ctes>;

      template <typename Expression>
      static constexpr bool _no_unknown_tables = _all_provided_tables::contains_all(required_tables_of_t<Expression>{});

      // workaround for msvc bug https://connect.microsoft.com/VisualStudio/Feedback/Details/2086629
      //	  template <typename... Expressions>
      //      using _no_unknown_aggregates =
      //          logic::any<_all_provided_aggregates::size::value == 0,
      //                       logic::all<is_aggregate_expression_t<_all_provided_aggregates,
      //                       Expressions>::value...>::value>;
      template <typename... Expressions>
      using _no_unknown_aggregates =
          logic::any<_all_provided_aggregates::empty(),
                       logic::all<detail::is_aggregate_expression_impl<_all_provided_aggregates,
                                                                         Expressions>::type::value...>::value>;

      template <typename... Expressions>
      using _all_aggregates = logic::any<logic::all<
          detail::is_aggregate_expression_impl<_all_provided_aggregates, Expressions>::type::value...>::value>;

      template <typename... Expressions>
      using _no_aggregates = logic::any<logic::all<
          detail::is_non_aggregate_expression_impl<_all_provided_aggregates, Expressions>::type::value...>::value>;

      template <template <typename> class Predicate>
      using any_t = logic::any<Predicate<Policies>::value...>;

      using _result_type_provider = detail::get_last_if_t<is_result_clause, noop, Policies...>;

      struct _result_methods_t : public _result_type_provider::template _result_methods_t<_statement_t>
      {
      };

      // A select can be used as a pseudo table if
      //   - at least one column is selected
      //   - the select is complete (leaks no table requirements or cte requirements)
      static constexpr bool _can_be_used_as_table()
      {
        return has_result_row<_statement_t>::value and _required_tables_of::empty() and
               _required_ctes_of::empty();
      }

      using _value_type =
          typename std::conditional<is_any_policy_missing<Policies...>(),
                                    no_value_t,  // if a required statement part is missing (e.g. columns in a select),
                                                 // then the statement cannot be used as a value
                                    value_type_of_t<_result_type_provider>>::type;

      using _nodes = detail::type_vector<>;
#warning: maybe need to make value type optional
      /*
      using _can_be_null = logic::any<can_be_null_t<_result_type_provider>::value,
                                        detail::make_intersect_set_t<required_tables_of_t<_result_type_provider>,
                                                                     _all_provided_optional_tables>::size::value != 0>;
                                        */
      using _parameters = detail::type_vector_cat_t<parameters_of_t<Policies>...>;
      // required_tables and _required_ctes are defined above

      using _cte_check =
          typename std::conditional<_required_ctes_of::empty(), consistent_t, assert_no_unknown_ctes_t>::type;
      using _table_check =
          typename std::conditional<_required_tables_of::empty(), consistent_t, assert_no_unknown_tables_t>::type;
      using _parameter_check = typename std::
          conditional<_parameters::empty(), consistent_t, assert_no_parameters_t>::type;
    };

  }  // namespace detail

  template <typename... Policies>
  struct value_type_of<detail::statement_policies_t<Policies...>>
  {
    using type = typename detail::statement_policies_t<Policies...>::_value_type;
  };

  template <typename... Policies>
  struct statement_t : public Policies::template _base_t<detail::statement_policies_t<Policies...>>...,
#warning: reactivate
  /*
                       public expression_operators<statement_t<Policies...>,
                                                   value_type_of_t<detail::statement_policies_t<Policies...>>>,
                                                   */
                       public detail::statement_policies_t<Policies...>::_result_methods_t
  {
    using _policies_t = typename detail::statement_policies_t<Policies...>;

    using _consistency_check =
        detail::get_first_if<is_inconsistent_t,
                             consistent_t,
                             typename Policies::template _base_t<_policies_t>::_consistency_check...,
                             typename _policies_t::_table_check>;

    using _run_check = detail::get_first_if<is_inconsistent_t,
                                            consistent_t,
                                            typename _policies_t::_parameter_check,
                                            typename _policies_t::_cte_check,
                                            typename Policies::template _base_t<_policies_t>::_consistency_check...,
                                            typename _policies_t::_table_check>;

    using _prepare_check = detail::get_first_if<is_inconsistent_t,
                                                consistent_t,
                                                typename _policies_t::_cte_check,
                                                typename Policies::template _base_t<_policies_t>::_consistency_check...,
                                                typename _policies_t::_table_check>;

    using _result_type_provider = typename _policies_t::_result_type_provider;
    template <typename Composite>
    using _result_methods_t = typename _result_type_provider::template _result_methods_t<Composite>;

    using _traits =
        make_traits<value_type_of_t<_policies_t>,
                    tag::is_statement,
                    tag_if<tag::is_select, logic::any<is_select_t<Policies>::value...>::value>,
                    tag_if<tag::is_expression, is_expression_t<_policies_t>::value>,
                    tag_if<tag::is_selectable, is_expression_t<_policies_t>::value>
#warning: reactivate
                    //,tag_if<tag::is_return_value, logic::none<is_noop_t<_result_type_provider>::value>::value>
                      >;
    using _name_tag_of = name_tag_of<_result_type_provider>;
    using _nodes = detail::type_vector<_policies_t>;
    using _provided_optional_tables = typename _policies_t::_all_provided_optional_tables;

    // Constructors
    statement_t() = default;

    // workaround for msvc bug https://connect.microsoft.com/VisualStudio/Feedback/Details/2173269
    //	template <typename Statement, typename Term>
    //	statement_t(Statement statement, Term term)
    //		: Policies::template _base_t<_policies_t>{typename Policies::template _impl_t<_policies_t>{
    //		detail::pick_arg<typename Policies::template _base_t<_policies_t>>(statement, term)}}...
    //	{
    //	}
    template <typename Statement, typename Term>
    statement_t(Statement statement, Term term)
        : Policies::template _base_t<_policies_t>(
              detail::pick_arg<Policies>(statement, term))...
    {
    }

    statement_t(const statement_t& r) = default;
    statement_t(statement_t&& r) = default;
    statement_t& operator=(const statement_t& r) = default;
    statement_t& operator=(statement_t&& r) = default;
    ~statement_t() = default;

    static constexpr size_t _get_static_no_of_parameters()
    {
      return parameters_of<statement_t>::size;
    }

    size_t _get_no_of_parameters() const
    {
      return _get_static_no_of_parameters();
    }

    static constexpr bool _can_be_used_as_table()
    {
      return _policies_t::_can_be_used_as_table();
    }

    template <typename Database>
    auto _run(Database& db) const -> decltype(std::declval<_result_methods_t<statement_t>>()._run(db))
    {
      _run_check::verify();
      return _result_methods_t<statement_t>::_run(db);
    }

    template <typename Database>
    auto _prepare(Database& db) const -> decltype(std::declval<_result_methods_t<statement_t>>()._prepare(db))
    {
      _prepare_check::verify();
      return _result_methods_t<statement_t>::_prepare(db);
    }
  };

  template<typename... Policies>
    struct value_type_of<statement_t<Policies...>> : public value_type_of<typename detail::statement_policies_t<Policies...>> {};
  template<typename... Policies>
    struct name_tag_of<statement_t<Policies...>> : public statement_t<Policies...>::_name_tag_of {};

  template <typename... Policies>
  struct nodes_of<statement_t<Policies...>>
  {
    using type = typename detail::type_vector<Policies...>;
  };

  template <typename... Policies>
  struct required_tables_of<statement_t<Policies...>>
  {
    using type = typename detail::statement_policies_t<Policies...>::_required_tables_of;
  };

  template <typename... Policies>
    struct known_aggregate_columns_of<statement_t<Policies...>>
    {
      using type = detail::type_vector_cat_t<known_aggregate_columns_of_t<Policies>...>;
    };

  template <typename... Policies>
  struct requires_parentheses<statement_t<Policies...>> : public std::true_type {};

  template <typename Context, typename... Policies>
  auto to_sql_string(Context& context, const statement_t<Policies...>& t) -> std::string
  {
    using P = detail::statement_policies_t<Policies...>;

    auto result = std::string{};
    using swallow = int[];
    (void)swallow{
        0,
        (result += to_sql_string(context, static_cast<const typename Policies::template _base_t<P>&>(t)._data), 0)...};

    return result;
  }

  template <typename NameData, typename Tag = tag::is_noop>
  struct statement_name_t
  {
    using _traits = make_traits<no_value_t, Tag>;

    using _data_t = NameData;

    // Base template to be inherited by the statement
    template <typename Policies>
    struct _base_t
    {
      _base_t() = default;
      _base_t(_data_t data) : _data{std::move(data)}
      {
      }

      _data_t _data;

      using _consistency_check = consistent_t;
    };
  };
}  // namespace sqlpp
