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

#include <tuple>

#include <sqlpp23/core/basic/table.h>
#include <sqlpp23/core/clause/expression_static_check.h>
#include <sqlpp23/core/clause/select_as.h>
#include <sqlpp23/core/clause/select_column_traits.h>
#include <sqlpp23/core/clause/select_columns_aggregate_check.h>
#include <sqlpp23/core/concepts.h>
#include <sqlpp23/core/database/prepared_select.h>
#include <sqlpp23/core/detail/flat_tuple.h>
#include <sqlpp23/core/detail/type_set.h>
#include <sqlpp23/core/field_spec.h>
#include <sqlpp23/core/operator/as_expression.h>
#include <sqlpp23/core/query/dynamic.h>
#include <sqlpp23/core/query/result_row.h>
#include <sqlpp23/core/query/statement.h>
#include <sqlpp23/core/query/statement_handler.h>
#include <sqlpp23/core/reader.h>
#include <sqlpp23/core/tuple_to_sql_string.h>

namespace sqlpp {
class assert_no_unknown_tables_in_selected_columns_t
    : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    SQLPP_STATIC_ASSERT(wrong<T...>,
                        "at least one selected column requires a table "
                        "which is otherwise not known in the statement");
  }
};

class assert_no_unknown_static_tables_in_selected_columns_t
    : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    SQLPP_STATIC_ASSERT(
        wrong<T...>,
        "at least one selected column statically requires a table which is "
        "otherwise not known dynamically in the statement");
  }
};

// SELECTED COLUMNS
template <typename... Columns>
struct select_column_list_t {
  select_column_list_t(std::tuple<Columns...> columns)
      : _columns(std::move(columns)) {}

  select_column_list_t(const select_column_list_t&) = default;
  select_column_list_t(select_column_list_t&&) = default;
  select_column_list_t& operator=(const select_column_list_t&) = default;
  select_column_list_t& operator=(select_column_list_t&&) = default;
  ~select_column_list_t() = default;

 private:
  friend reader_t;
  std::tuple<Columns...> _columns;
};

template <typename Context, typename... Columns>
auto to_sql_string(Context& context, const select_column_list_t<Columns...>& t)
    -> std::string {
  // dynamic(false, foo.id) -> NULL as id
  // dynamic(false, foo.id).as(cheesecake) -> NULL AS cheesecake
  // max(something) -> max(something) as _max
  // max(something.as(cheesecake) -> max(something) AS cheesecake
  return tuple_to_sql_string(context, read.columns(t),
                             tuple_operand_select_column{", "});
}

template <typename... Columns>
struct is_clause<select_column_list_t<Columns...>> : public std::true_type {};

template <typename... Columns>
struct has_result_row<select_column_list_t<Columns...>>
    : public std::true_type {};

template <typename Statement, typename... Columns>
struct result_row_of<Statement, select_column_list_t<Columns...>> {
  using type = result_row_t<make_field_spec_t<Statement, Columns>...>;
};

template <typename... Columns>
struct select_result_methods_t {
  template <typename Statement, typename NameTagProvider>
  auto as(this Statement&& self, const NameTagProvider&)
      -> select_as_t<std::decay_t<Statement>,
                     name_tag_of_t<NameTagProvider>,
                     make_field_spec_t<std::decay_t<Statement>, Columns>...> {
    // This ensures that the sub select is free of table/CTE dependencies and
    // consistent.
    check_prepare_consistency(self).verify();

    using table =
        select_as_t<std::decay_t<Statement>, name_tag_of_t<NameTagProvider>,
                    make_field_spec_t<std::decay_t<Statement>, Columns>...>;
    return table(std::forward<Statement>(self));
  }

 private:
  friend class statement_handler_t;

  // Execute
  template <typename Statement, typename Db>
  auto _run(this Statement&& self, Db& db) -> result_t<
      decltype(statement_handler_t{}.select(std::forward<Statement>(self), db)),
      result_row_t<make_field_spec_t<std::decay_t<Statement>, Columns>...>> {
    return {statement_handler_t{}.select(std::forward<Statement>(self), db)};
  }

  // Prepare
  template <typename Statement, typename Db>
  auto _prepare(this Statement&& self, Db& db)
      -> prepared_select_t<Db, std::decay_t<Statement>> {
    return {{},
            statement_handler_t{}.prepare_select(std::forward<Statement>(self),
                                                 db)};
  }
};

template <typename... Columns>
struct no_of_result_columns<select_column_list_t<Columns...>> {
  static constexpr size_t value = sizeof...(Columns);
};

template <typename... Columns>
struct result_methods_of<select_column_list_t<Columns...>> {
  using type = select_result_methods_t<Columns...>;
};

template <typename Statement, typename... Columns>
struct consistency_check<Statement, select_column_list_t<Columns...>> {
  using AC = typename Statement::_all_provided_aggregates;
  static constexpr bool has_group_by = not AC::empty();

  using type = static_combined_check_t<
      detail::select_columns_aggregate_check_t<
          has_group_by,
          Statement,
          detail::remove_as_from_select_column_t<Columns>...>,
      detail::expression_static_check_t<
          Statement,
          detail::remove_as_from_select_column_t<Columns>,
          assert_no_unknown_static_tables_in_selected_columns_t>...>;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename Statement, typename... Columns>
struct prepare_check<Statement, select_column_list_t<Columns...>> {
  using type = static_combined_check_t<
      static_check_t<Statement::template _no_unknown_tables<
                         select_column_list_t<Columns...>>,
                     assert_no_unknown_tables_in_selected_columns_t>,
      static_check_t<Statement::template _no_unknown_static_tables<
                         select_column_list_t<Columns...>>,
                     assert_no_unknown_static_tables_in_selected_columns_t>>;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename Column>
struct data_type_of<select_column_list_t<Column>>
    : public select_column_data_type_of<Column> {};

template <typename... Columns>
struct is_result_clause<select_column_list_t<Columns...>>
    : public std::true_type {};

template <typename... Columns>
struct nodes_of<select_column_list_t<Columns...>> {
  using type = detail::type_vector<Columns...>;
};

class assert_columns_selected_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    SQLPP_STATIC_ASSERT(wrong<T...>, "selecting columns required");
  }
};

template <typename ColumnTuple>
struct make_select_column_list;
template <typename... Columns>
struct make_select_column_list<std::tuple<Columns...>> {
  using type = select_column_list_t<Columns...>;
};
template <typename... Columns>
using make_select_column_list_t =
    typename make_select_column_list<detail::flat_tuple_t<Columns...>>::type;

struct no_select_column_list_t {
  template <typename Statement, DynamicSelectColumn... _Columns>
    requires(sizeof...(_Columns) > 0)
  auto columns(this Statement&& self, _Columns... args) {
    return new_statement<no_select_column_list_t>(
        std::forward<Statement>(self),
        make_select_column_list_t<_Columns...>{
            std::tuple_cat(detail::tupelize(std::move(args))...)});
  }
};

template <typename Context>
auto to_sql_string(Context&, const no_select_column_list_t&) -> std::string {
  return "";
}

template <typename Statement>
struct consistency_check<Statement, no_select_column_list_t> {
  using type = assert_columns_selected_t;
  constexpr auto operator()() {
    return type{};
  }
};

template <DynamicSelectColumn... _Columns>
  requires(sizeof...(_Columns) > 0)
auto select_columns(_Columns... columns) {
  return statement_t<no_select_column_list_t>().columns(std::move(columns)...);
}

}  // namespace sqlpp
