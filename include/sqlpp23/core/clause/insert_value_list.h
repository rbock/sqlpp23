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
#include <sqlpp23/core/basic/table.h>
#include <sqlpp23/core/clause/insert_value.h>
#include <sqlpp23/core/clause/simple_column.h>
#include <sqlpp23/core/logic.h>
#include <sqlpp23/core/no_data.h>
#include <sqlpp23/core/operator/assign_expression.h>
#include <sqlpp23/core/query/dynamic.h>
#include <sqlpp23/core/query/statement.h>
#include <sqlpp23/core/reader.h>
#include <sqlpp23/core/tuple_to_sql_string.h>
#include <sqlpp23/core/type_traits.h>
#include <sqlpp23/core/wrapped_static_assert.h>

namespace sqlpp {
namespace detail {
template <typename Clauses, typename... Columns>
struct have_all_required_columns {
  static constexpr bool value =
      detail::make_type_set_t<Columns...>::contains_all(
          required_insert_columns_of_t<Clauses>{});
};

template <typename Clauses, typename... Assignments>
struct have_all_required_assignments {
  static constexpr bool value =
      have_all_required_columns<Clauses, lhs_t<Assignments>...>::value;
};

// Used to serialize left hand side of assignment tuple that should ignore
// dynamic elements.
struct tuple_lhs_assignment_operand_no_dynamic {
  template <typename Context, typename Lhs, typename Op, typename Rhs>
  auto operator()(Context& context,
                  const assign_expression<Lhs, Op, Rhs>&,
                  size_t) const -> std::string {
    const auto prefix = need_prefix ? std::string{separator} : std::string{};
    need_prefix = true;
    return prefix + name_to_sql_string(context, name_tag_of_t<Lhs>{});
  }

  template <typename Context, typename T>
  auto operator()(Context& context,
                  const sqlpp::dynamic_t<T>& t,
                  size_t index) const -> std::string {
    if (t.has_value()) {
      return operator()(context, t.value(), index);
    }
    return "";
  }

  std::string_view separator;
  mutable bool need_prefix = false;
};

// Used to serialize right hand side of assignment tuple that should ignore
// dynamic elements.
struct tuple_rhs_assignment_operand_no_dynamic {
  template <typename Context, typename Lhs, typename Op, typename Rhs>
  auto operator()(Context& context,
                  const assign_expression<Lhs, Op, Rhs>& t,
                  size_t) const -> std::string {
    const auto prefix = need_prefix ? std::string{separator} : std::string{};
    need_prefix = true;
    return prefix + operand_to_sql_string(context, t._r);
  }

  template <typename Context, typename T>
  auto operator()(Context& context,
                  const sqlpp::dynamic_t<T>& t,
                  size_t index) const -> std::string {
    if (t.has_value()) {
      return operator()(context, t.value(), index);
    }
    return "";
  }

  std::string_view separator;
  mutable bool need_prefix = false;
};

}  // namespace detail

struct insert_default_values_t {
  template <typename Context>
  friend auto to_sql_string(Context&, const insert_default_values_t&)
      -> std::string {
    return " DEFAULT VALUES";
  }
};

template <>
struct is_clause<insert_default_values_t> : public std::true_type {};

class assert_all_columns_have_default_value_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>,
                        "at least one column does not have a default value "
                        "(explicit default, NULL, or auto-increment)");
  }
};

class assert_all_required_columns_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>,
                        "at least one required column is missing in columns()");
  }
};

class assert_all_required_assignments_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>,
                        "at least one required column is missing in set()");
  }
};

template <typename Statement>
struct consistency_check<Statement, insert_default_values_t> {
  using type =
      std::conditional_t<required_insert_columns_of_t<Statement>::empty(),
                         consistent_t,
                         assert_all_columns_have_default_value_t>;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename... Assignments>
struct insert_set_t {
  insert_set_t(std::tuple<Assignments...> assignments)
      : _assignments(std::move(assignments)) {}

  insert_set_t(const insert_set_t&) = default;
  insert_set_t(insert_set_t&&) = default;
  insert_set_t& operator=(const insert_set_t&) = default;
  insert_set_t& operator=(insert_set_t&&) = default;
  ~insert_set_t() = default;

 private:
  friend reader_t;
  std::tuple<Assignments...> _assignments;
};

template <typename Context, typename... Assignments>
auto to_sql_string(Context& context, const insert_set_t<Assignments...>& t)
    -> std::string {
  auto result = std::string{" ("};
  result += tuple_to_sql_string(
      context, read.assignments(t),
      detail::tuple_lhs_assignment_operand_no_dynamic{", "});
  result += ") VALUES(";
  result += tuple_to_sql_string(
      context, read.assignments(t),
      detail::tuple_rhs_assignment_operand_no_dynamic{", "});
  result += ")";
  return result;
}

class assert_no_unknown_tables_in_insert_assignments_t
    : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>,
                        "at least one insert assignment requires a table "
                        "which is otherwise not known in the statement");
  }
};

template <typename... Assignments>
struct is_clause<insert_set_t<Assignments...>> : public std::true_type {};

template <typename Statement, typename... Assignments>
struct consistency_check<Statement, insert_set_t<Assignments...>> {
  using type = static_combined_check_t<
      static_check_t<
          Statement::template _no_unknown_tables<insert_set_t<Assignments...>>,
          assert_no_unknown_tables_in_insert_assignments_t>,
      static_check_t<
          detail::have_all_required_assignments<Statement,
                                                Assignments...>::value,
          assert_all_required_assignments_t>>;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename... Assignments>
struct nodes_of<insert_set_t<Assignments...>> {
  using type = detail::type_vector<Assignments...>;
};

template <typename Tuple, typename... Assignments>
concept CorrectAddValuesAssignments = requires(Assignments... assignments) {
  Tuple{make_insert_value_t<lhs_t<Assignments>>(get_rhs(assignments))...};
};

template <typename... Columns>
struct column_list_t {
  column_list_t(std::tuple<make_simple_column_t<Columns>...> columns)
      : _columns(std::move(columns)) {}
  column_list_t(const column_list_t&) = default;
  column_list_t(column_list_t&&) = default;
  column_list_t& operator=(const column_list_t&) = default;
  column_list_t& operator=(column_list_t&&) = default;
  ~column_list_t() = default;

  template <DynamicAssignment... Assignments>
    requires(
        std::is_same<std::tuple<Columns...>,
                     std::tuple<lhs_t<Assignments>...>>::value and
        CorrectAddValuesAssignments<std::tuple<make_insert_value_t<Columns>...>,
                                    Assignments...>)
  auto add_values(Assignments... assignments) -> void {
    _expressions.emplace_back(
        make_insert_value_t<lhs_t<Assignments>>(get_rhs(assignments))...);
  }

 private:
  friend reader_t;
  std::tuple<make_simple_column_t<Columns>...> _columns;
  using _value_tuple_t = std::tuple<make_insert_value_t<Columns>...>;
  std::vector<_value_tuple_t> _expressions;
};

template <typename Context, typename... Columns>
auto to_sql_string(Context& context, const column_list_t<Columns...>& t)
    -> std::string {
  auto result = std::string{" ("};
  result += tuple_to_sql_string(context, read.columns(t),
                                tuple_operand_no_dynamic{", "});
  result += ")";
  bool first = true;
  for (const auto& row : read.expressions(t)) {
    if (first) {
      result += " VALUES ";
      first = false;
    } else {
      result += ", ";
    }
    result += '(';
    result += tuple_to_sql_string(context, row, tuple_operand_no_dynamic{", "});
    result += ')';
  }

  return result;
}

class assert_no_unknown_tables_in_column_list_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>,
                        "at least one column requires a table which is "
                        "otherwise not known in the statement");
  }
};

template <typename... Columns>
struct is_clause<column_list_t<Columns...>> : public std::true_type {};

template <typename Statement, typename... Columns>
struct consistency_check<Statement, column_list_t<Columns...>> {
  using type = static_combined_check_t<
      static_check_t<
          Statement::template _no_unknown_tables<column_list_t<Columns...>>,
          assert_no_unknown_tables_in_column_list_t>,
      static_check_t<
          detail::have_all_required_columns<Statement, Columns...>::value,
          assert_all_required_columns_t>>;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename... Columns>
struct nodes_of<column_list_t<Columns...>> {
  using type = detail::type_vector<Columns...>;
};

class assert_insert_values_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(
        wrong<T...>,
        "insert values required, e.g. set(...) or default_values()");
  }
};

// NO INSERT COLUMNS/VALUES YET
struct no_insert_value_list_t {
  template <typename Statement>
  auto default_values(this Statement&& self) {
    return new_statement<no_insert_value_list_t>(std::forward<Statement>(self),
                                                 insert_default_values_t{});
  }

  template <typename Statement, DynamicColumn... Columns>
    requires(
        sizeof...(Columns) > 0 and
        logic::none<is_const<Columns>::value...>::value and
        detail::are_unique<remove_dynamic_t<Columns>...>::value and
        detail::are_same<typename remove_dynamic_t<Columns>::_table...>::value)
  auto columns(this Statement&& self, Columns... cols) {
    return new_statement<no_insert_value_list_t>(
        std::forward<Statement>(self),
        column_list_t<Columns...>{std::make_tuple(std::move(cols)...)});
  }

  template <typename Statement, DynamicAssignment... Assignments>
    requires(
        sizeof...(Assignments) > 0 and
        detail::are_unique<lhs_t<remove_dynamic_t<Assignments>>...>::value and
        detail::are_same<
            typename lhs_t<remove_dynamic_t<Assignments>>::_table...>::value)
  auto set(this Statement&& self, Assignments... assignments) {
    return new_statement<no_insert_value_list_t>(
        std::forward<Statement>(self),
        insert_set_t<Assignments...>{
            std::make_tuple(std::move(assignments)...)});
  }
};

template <typename Context>
auto to_sql_string(Context&, const no_insert_value_list_t&) -> std::string {
  return "";
}

template <typename Statement>
struct consistency_check<Statement, no_insert_value_list_t> {
  using type = assert_insert_values_t;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename... Assignments>
auto insert_default_values() {
  return statement_t<no_insert_value_list_t>().default_values();
}

template <DynamicAssignment... Assignments>
    requires(
        sizeof...(Assignments) > 0 and
        detail::are_unique<lhs_t<remove_dynamic_t<Assignments>>...>::value and
        detail::are_same<
            typename lhs_t<remove_dynamic_t<Assignments>>::_table...>::value)
auto insert_set(Assignments... assignments) {
  return statement_t<no_insert_value_list_t>().set(assignments...);
}

template <DynamicColumn... Columns>
  requires(
      sizeof...(Columns) > 0 and
      logic::none<is_const<Columns>::value...>::value and
      detail::are_unique<remove_dynamic_t<Columns>...>::value and
      detail::are_same<typename remove_dynamic_t<Columns>::_table...>::value)
auto insert_columns(Columns... cols) {
  return statement_t<no_insert_value_list_t>().columns(cols...);
}
}  // namespace sqlpp
