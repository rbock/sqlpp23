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

#include <sqlpp23/core/clause/expression_static_check.h>
#include <sqlpp23/core/concepts.h>
#include <sqlpp23/core/logic.h>
#include <sqlpp23/core/query/statement.h>
#include <sqlpp23/core/reader.h>
#include <sqlpp23/core/tuple_to_sql_string.h>
#include <sqlpp23/core/type_traits.h>
#include <tuple>

namespace sqlpp {
template <typename... Expressions>
struct group_by_t {
  group_by_t(std::tuple<Expressions...> expressions)
      : _expressions(std::move(expressions)) {}

  group_by_t(const group_by_t&) = default;
  group_by_t(group_by_t&&) = default;
  group_by_t& operator=(const group_by_t&) = default;
  group_by_t& operator=(group_by_t&&) = default;
  ~group_by_t() = default;

 private:
  friend reader_t;
  std::tuple<Expressions...> _expressions;
};

template <typename Context, typename... Expressions>
auto to_sql_string(Context& context, const group_by_t<Expressions...>& t)
    -> std::string {
  return dynamic_tuple_clause_to_sql_string(context, "GROUP BY",
                                            read.expressions(t));
}

class assert_no_unknown_tables_in_group_by_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>,
                        "at least one group-by expression requires a table "
                        "which is otherwise not known in the statement");
  }
};

class assert_no_unknown_static_tables_in_group_by_t
    : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(
        wrong<T...>,
        "at least one group-by expression statically requires a table which is "
        "only known dynamically in the statement");
  }
};

template <typename... Expressions>
struct is_clause<group_by_t<Expressions...>> : public std::true_type {};

template <typename Statement, typename... Expressions>
struct consistency_check<Statement, group_by_t<Expressions...>> {
  using type = detail::expression_static_check_t<
      Statement,
      group_by_t<Expressions...>,
      assert_no_unknown_static_tables_in_group_by_t>;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename Statement, typename... Expressions>
struct prepare_check<Statement, group_by_t<Expressions...>> {
  using type = static_combined_check_t<
      static_check_t<
          Statement::template _no_unknown_tables<group_by_t<Expressions...>>,
          assert_no_unknown_tables_in_group_by_t>,
      static_check_t<Statement::template _no_unknown_static_tables<
                         group_by_t<Expressions...>>,
                     assert_no_unknown_static_tables_in_group_by_t>>;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename... Expressions>
struct known_aggregate_columns_of<group_by_t<Expressions...>> {
  using type =
      detail::type_set<remove_dynamic_t<Expressions>...>;
};

namespace detail {
template <typename Column>
struct make_static_aggregate_column_set {
  using type = detail::type_set<Column>;
};
template <typename Column>
struct make_static_aggregate_column_set<dynamic_t<Column>> {
  using type = detail::type_set<>;
};
template <typename Column>
using make_static_aggregate_column_set_t =
    typename make_static_aggregate_column_set<Column>::type;
}  // namespace detail

template <typename... Expressions>
struct known_static_aggregate_columns_of<group_by_t<Expressions...>> {
  using type = detail::make_joined_set_t<
      detail::make_static_aggregate_column_set_t<Expressions>...>;
};

template <typename... Expressions>
struct nodes_of<group_by_t<Expressions...>> {
  using type = detail::type_vector<Expressions...>;
};

// NO GROUP BY YET
struct no_group_by_t {
  template <typename Statement, DynamicValue... Expressions>
    requires(sizeof...(Expressions) > 0 and
             logic::none<contains_aggregate_function<
                 remove_dynamic_t<Expressions>>::value...>::value)
  auto group_by(this Statement&& self, Expressions... expressions) {
    return new_statement<no_group_by_t>(
        std::forward<Statement>(self),
        group_by_t<Expressions...>{std::make_tuple(std::move(expressions)...)});
  }
};

template <typename Context>
auto to_sql_string(Context&, const no_group_by_t&) -> std::string {
  return "";
}

template <typename Statement>
struct consistency_check<Statement, no_group_by_t> {
  using type = consistent_t;
  constexpr auto operator()() {
    return type{};
  }
};

template <DynamicValue... Expressions>
    requires(sizeof...(Expressions) > 0 and
             logic::none<contains_aggregate_function<
                 remove_dynamic_t<Expressions>>::value...>::value)
auto group_by(Expressions... expressions) {
  return statement_t<no_group_by_t>{}.group_by(std::move(expressions)...);
}

}  // namespace sqlpp
