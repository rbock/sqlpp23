#pragma once

/**
 * Copyright © 2014-2015, Matthijs Möhlmann
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
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

#include <sqlpp23/core/basic/table.h>
#include <sqlpp23/core/clause/select_as.h>
#include <sqlpp23/core/clause/select_column_traits.h>
#include <sqlpp23/core/detail/flat_tuple.h>
#include <sqlpp23/core/detail/type_set.h>
#include <sqlpp23/core/field_spec.h>
#include <sqlpp23/core/noop.h>
#include <sqlpp23/core/query/result_row.h>
#include <sqlpp23/core/query/statement_handler.h>
#include <sqlpp23/core/reader.h>
#include <sqlpp23/core/tuple_to_sql_string.h>
#include <tuple>

namespace sqlpp {
// RETURNING is used in DELETE, INSERT, and UPDATE statements in
// * PostgreSQL
// * sqlite3

class assert_no_unknown_tables_in_returning_columns_t
    : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>,
                        "at least one returning column requires a table "
                        "which is otherwise not known in the statement");
  }
};

class assert_returning_columns_contain_no_aggregates_t
    : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(
        wrong<T...>, "returning columns must not contain aggregate functions");
  }
};

template <typename... Columns>
struct returning_t {
  returning_t(std::tuple<Columns...> columns) : _columns(std::move(columns)) {}
  returning_t(const returning_t&) = default;
  returning_t(returning_t&&) = default;
  returning_t& operator=(const returning_t&) = default;
  returning_t& operator=(returning_t&&) = default;
  ~returning_t() = default;

 private:
  friend ::sqlpp::reader_t;
  std::tuple<Columns...> _columns;
};

template <typename Context, typename... Columns>
auto to_sql_string(Context& context, const returning_t<Columns...>& t)
    -> std::string {
  return " RETURNING " + tuple_to_sql_string(context, read.columns(t),
                                             tuple_operand_select_column{", "});
}

template <typename... Columns>
struct returning_column_list_result_methods_t {
  template <typename Statement, typename NameTagProvider>
  auto as(this Statement&& self, const NameTagProvider&)
      -> select_as_t<std::decay_t<Statement>,
                     name_tag_of_t<NameTagProvider>,
                     make_field_spec_t<std::decay_t<Statement>, Columns>...> {
    check_prepare_consistency(self).verify();
    using table =
        select_as_t<std::decay_t<Statement>, name_tag_of_t<NameTagProvider>,
                    make_field_spec_t<std::decay_t<Statement>, Columns>...>;
    return table(std::forward<Statement>(self));
  }

 private:
  friend class sqlpp::statement_handler_t;

  // Execute
  template <typename Statement, typename Db>
  auto _run(this Statement&& self, Db& db) -> result_t<
      decltype(sqlpp::statement_handler_t{}
                   .select(std::declval<std::decay_t<Statement>>(), db)),
      result_row_t<make_field_spec_t<std::decay_t<Statement>, Columns>...>> {
    return {statement_handler_t{}.select(std::forward<Statement>(self), db)};
  }

  // Prepare
  template <typename Statement, typename Db>
  auto _prepare(this Statement&& self, Db& db)
      -> prepared_select_t<Db, std::decay_t<Statement>> {
    return {make_parameter_list_t<std::decay_t<Statement>>{},
            statement_handler_t{}.prepare_select(std::forward<Statement>(self),
                                                 db)};
  }
};

template <typename... Columns>
struct no_of_result_columns<returning_t<Columns...>> {
  static constexpr size_t value = sizeof...(Columns);
};

template <typename... Columns>
struct has_result_row<returning_t<Columns...>> : public std::true_type {};

template <typename Statement, typename... Columns>
struct result_row_of<Statement, returning_t<Columns...>> {
  using type = result_row_t<make_field_spec_t<Statement, Columns>...>;
};

template <typename... Columns>
struct result_methods_of<returning_t<Columns...>> {
  using type = returning_column_list_result_methods_t<Columns...>;
};

template <typename... Columns>
struct nodes_of<returning_t<Columns...>> {
  using type = detail::type_vector<Columns...>;
};

template <typename... Columns>
struct is_clause<returning_t<Columns...>> : public std::true_type {};

template <typename Statement, typename... Columns>
struct consistency_check<Statement, returning_t<Columns...>> {
  using type = static_check_t<
      not contains_aggregate_function<returning_t<Columns...>>::value,
      assert_returning_columns_contain_no_aggregates_t>;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename Statement, typename... Columns>
struct prepare_check<Statement, returning_t<Columns...>> {
  using type = static_check_t<
      Statement::template _no_unknown_tables<returning_t<Columns...>>,
      assert_no_unknown_tables_in_returning_columns_t>;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename Column>
struct data_type_of<returning_t<Column>> : public data_type_of<Column> {};

template <typename Column>
struct name_tag_of<returning_t<Column>> : public name_tag_of<Column> {};

template <typename... Column>
struct is_result_clause<returning_t<Column...>> : public std::true_type {};

template <typename ColumnTuple>
struct make_returning_column_list;

template <typename... Columns>
struct make_returning_column_list<std::tuple<Columns...>> {
  using type = returning_t<Columns...>;
};

template <typename... Columns>
using make_returning_t = typename make_returning_column_list<
    sqlpp::detail::flat_tuple_t<is_select_column, Columns...>>::type;

struct no_returning_t {
  template <typename Statement, DynamicSelectColumn... Columns>
    requires(sizeof...(Columns) > 0 and
             select_columns_have_values<Columns...>::value and
             select_columns_have_names<Columns...>::value)
  auto returning(this Statement&& self, Columns... columns) {
    return new_statement<no_returning_t>(
        std::forward<Statement>(self),
        make_returning_t<Columns...>{std::tuple_cat(
            sqlpp::detail::tupelize<is_select_column>(std::move(columns))...)});
  }
};

template <typename Context>
auto to_sql_string(Context&, const no_returning_t&) -> std::string {
  return "";
}

template <typename Statement>
struct consistency_check<Statement, no_returning_t> {
  using type = consistent_t;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename... Columns>
    requires(sizeof...(Columns) > 0 and
             select_columns_have_values<Columns...>::value and
             select_columns_have_names<Columns...>::value)
auto returning(Columns... columns)
    -> decltype(statement_t<no_returning_t>{}.returning(
        std::move(columns)...)) {
  return statement_t<no_returning_t>{}.returning(std::move(columns)...);
}

}  // namespace sqlpp
