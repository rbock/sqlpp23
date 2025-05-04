/*
 * Copyright (c) 2024, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
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

#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/tests/core/tables.h>

namespace {
template <typename... Expressions>
concept can_call_insert_set_with_standalone =
    requires(Expressions... expressions) { sqlpp::insert_set(expressions...); };
template <typename... Expressions>
concept can_call_insert_set_with_in_statement =
    requires(Expressions... expressions) {
      sqlpp::statement_t<sqlpp::no_insert_value_list_t>{}.set(expressions...);
    };

template <typename... Expressions>
concept can_call_insert_set_with =
    can_call_insert_set_with_standalone<Expressions...> and
    can_call_insert_set_with_in_statement<Expressions...>;

template <typename... Expressions>
concept cannot_call_insert_set_with =
    not(can_call_insert_set_with_standalone<Expressions...> or
        can_call_insert_set_with_in_statement<Expressions...>);

template <typename... Expressions>
concept can_call_insert_columns_with_standalone = requires(
    Expressions... expressions) { sqlpp::insert_columns(expressions...); };
template <typename... Expressions>
concept can_call_insert_columns_with_in_statement = requires(
    Expressions... expressions) {
  sqlpp::statement_t<sqlpp::no_insert_value_list_t>{}.columns(expressions...);
};

template <typename... Expressions>
concept can_call_insert_columns_with =
    can_call_insert_columns_with_standalone<Expressions...> and
    can_call_insert_columns_with_in_statement<Expressions...>;

template <typename... Expressions>
concept cannot_call_insert_columns_with =
    not(can_call_insert_columns_with_standalone<Expressions...> or
        can_call_insert_columns_with_in_statement<Expressions...>);

template <typename Statement, typename... Expressions>
concept can_call_add_values_with = requires(Statement statement, Expressions... expressions) {
  statement.add_values(expressions...);
};

template <typename Statement, typename... Expressions>
concept cannot_call_add_values_with =
    not can_call_add_values_with<Statement, Expressions...>;

}  // namespace

int main() {
  const auto foo = test::TabFoo{};
  const auto bar = test::TabBar{};

  // Confirming the required columns of TabBar.
  static_assert(std::is_same<sqlpp::required_insert_columns_of_t<test::TabBar>,
                             sqlpp::detail::type_set<sqlpp::column_t<
                                 test::TabBar, test::TabBar_::BoolNn>>>::value,
                "");

  // -------------------------
  // insert_into(tab).set(...)
  // -------------------------

  // insert_into(table).set(<non arguments>) is inconsistent and cannot be
  // constructed.
  static_assert(cannot_call_insert_set_with<>);

  // insert_set(<arguments including non-assignments>) is inconsistent and
  // cannot be constructed.
  static_assert(can_call_insert_set_with<decltype(bar.intN = 7)>);
  static_assert(cannot_call_insert_set_with<decltype(bar.intN == 7)>);
  static_assert(cannot_call_insert_set_with<decltype(bar.intN = 7),
                                            decltype(bar.boolNn)>);

  // insert_into(table).set(<arguments including non-assignments>) is
  // inconsistent and cannot be constructed.
  static_assert(cannot_call_insert_set_with<decltype(bar.boolNn = true),
                                            decltype(bar.boolNn = false)>);
  static_assert(cannot_call_insert_set_with<decltype(bar.boolNn = true),
                                            decltype(dynamic(false, bar.boolNn = false))>);

  // insert_into(table).set(<assignments from more than one table>) is
  // inconsistent and cannot be constructed.
  static_assert(cannot_call_insert_set_with<decltype(foo.intN = 7),
                                            decltype(bar.boolNn = false)>);
  static_assert(cannot_call_insert_set_with<decltype(dynamic(false, foo.intN = 7)),
                                            decltype(bar.boolNn = false)>);

  // insert_into(table).set(<not all required columns>) is inconsistent but can
  // be constructed (check can only run later)
  {
    auto i = insert_into(bar).set(bar.intN = sqlpp::default_value);
    using I = decltype(i);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<I>,
                               sqlpp::assert_all_required_assignments_t>::value,
                  "");
  }
  {
    auto i = insert_into(bar).set(dynamic(true, bar.intN = sqlpp::default_value));
    using I = decltype(i);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<I>,
                               sqlpp::assert_all_required_assignments_t>::value,
                  "");
  }

  // insert_into(table).set(<dynamic required columns>) is also inconsistent but
  // can be constructed (check can only run later)
  {
    auto i = insert_into(bar).set(dynamic(true, bar.boolNn = true));
    using I = decltype(i);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<I>,
                               sqlpp::assert_all_required_assignments_t>::value,
                  "");
  }

  // -------------------------
  // insert_into(tab).columns(...)
  // -------------------------

  // insert_into(table).columns(<non arguments>) is inconsistent and cannot be
  // constructed.
  static_assert(cannot_call_insert_columns_with<>);

  // insert_into(table).columns(<arguments including non-columns>) is
  // inconsistent and cannot be constructed.
  static_assert(can_call_insert_columns_with<decltype(bar.intN)>);
  static_assert(cannot_call_insert_columns_with<decltype(bar.intN = 7)>);

  // insert_into(table).columns(duplicate columns>) is
  // inconsistent and cannot be constructed.
  static_assert(
      cannot_call_insert_columns_with<decltype(bar.boolNn), decltype(bar.intN),
                                      decltype(bar.boolNn)>);
  static_assert(
      cannot_call_insert_columns_with<decltype(bar.boolNn), decltype(bar.intN),
                                      decltype(dynamic(false, bar.boolNn))>);

  // insert_into(table).columns(<columns from more than one table>) is
  // inconsistent and cannot be constructed.
  static_assert(
      cannot_call_insert_columns_with<decltype(bar.boolNn), decltype(foo.intN)>);
  static_assert(
      cannot_call_insert_columns_with<decltype(dynamic(false, bar.boolNn)),
                                      decltype(foo.intN)>);

  // insert_into(table).columns(<not all required columns>) is inconsistent but
  // can be constructed (check can only run later)
  {
    auto i = insert_into(bar).columns(bar.intN);
    using I = decltype(i);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<I>,
                               sqlpp::assert_all_required_columns_t>::value,
                  "");
  }
  {
    auto i = insert_into(bar).columns(dynamic(true, bar.intN));
    using I = decltype(i);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<I>,
                               sqlpp::assert_all_required_columns_t>::value,
                  "");
  }

  // insert_into(table).columns(<dynamic required columns>) is also inconsistent
  // but can be constructed (check can only run later)
  {
    auto i = insert_into(bar).columns(dynamic(true, bar.boolNn));
    using I = decltype(i);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<I>,
                               sqlpp::assert_all_required_columns_t>::value,
                  "");
  }

  // -------------------------
  // insert_into(tab).columns(...).add_value(...)
  // -------------------------
  {
    auto i = insert_into(bar).columns(bar.intN, bar.boolNn);
    using I = decltype(i);

    // OK, correct assignments
    static_assert(can_call_add_values_with<I, decltype(bar.intN = 7),
                                           decltype(bar.boolNn = true)>);
    static_assert(
        can_call_add_values_with<I, decltype(bar.intN = sqlpp::default_value),
                                 decltype(bar.boolNn = true)>);

    // Not OK, missing assignment
    static_assert(cannot_call_add_values_with<I, decltype(bar.boolNn = true)>);
    static_assert(cannot_call_add_values_with<I, decltype(bar.intN = 7)>);
    static_assert(
        cannot_call_add_values_with<I,
                                    decltype(bar.intN = sqlpp::default_value)>);

    // Not OK, cannot assign expressions
    static_assert(
        cannot_call_add_values_with<I,
                                    decltype(bar.intN = sqlpp::default_value),
                                    decltype(bar.boolNn = not bar.boolNn)>);

    // Not OK, cannot assign parameters
    static_assert(
        cannot_call_add_values_with<I, decltype(bar.intN = parameter(bar.intN)),
                                    decltype(bar.boolNn = true)>);
    static_assert(cannot_call_add_values_with<
                  I, decltype(bar.intN = sqlpp::default_value),
                  decltype(bar.boolNn = parameter(bar.boolNn))>);

    // Not OK, cannot assign named values
    static_assert(
        cannot_call_add_values_with<I,
                                    decltype(bar.intN = sqlpp::default_value),
                                    decltype(bar.boolNn = bar.boolNn)>);
  }

  // Dynamic columns
  {
    auto i = insert_into(bar).columns(dynamic(true, bar.intN), bar.boolNn);
    using I = decltype(i);

    i.add_values(dynamic(true, bar.intN = 7), bar.boolNn = true);
    static_assert(
        can_call_add_values_with<I, decltype(dynamic(true, bar.intN = 7)),
                                 decltype(bar.boolNn = true)>);

    // Not OK, compound expressions not allowed in values
    static_assert(cannot_call_add_values_with<
                  I, decltype(dynamic(true, bar.intN = bar.intN + 7)),
                  decltype(bar.boolNn = true)>);

    // Not OK, missing values
    static_assert(
        cannot_call_add_values_with<I, decltype(dynamic(true, bar.intN = 7))>);
    static_assert(cannot_call_add_values_with<I, decltype(bar.boolNn = true)>);
  }
}
