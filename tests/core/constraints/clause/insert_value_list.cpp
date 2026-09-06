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

#include <sqlpp26/tests/core/all.h>

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
  const auto foo = test::tab_foo{};
  const auto bar = test::tab_bar{};

  // Confirming the required columns of tab_bar.
  static_assert(
      sqlpp::required_insert_columns_of<test::tab_bar>::func() ==
      sqlpp::detail::make_type_info_set<sqlpp::column<test::tab_bar, 2>>());

  // -------------------------
  // insert_into(tab).set(...)
  // -------------------------

  // insert_into(table).set(<non arguments>) is inconsistent and cannot be
  // constructed.
  static_assert(cannot_call_insert_set_with<>);

  // insert_set(<arguments including non-assignments>) is inconsistent and
  // cannot be constructed.
  static_assert(can_call_insert_set_with<decltype(bar.int_n = 7)>);
  static_assert(cannot_call_insert_set_with<decltype(bar.int_n == 7)>);
  static_assert(cannot_call_insert_set_with<decltype(bar.int_n = 7),
                                            decltype(bar.bool_nn)>);

  // insert_into(table).set(<duplicate columns, including dynamic>) is
  // inconsistent and cannot be constructed.
  static_assert(cannot_call_insert_set_with<decltype(bar.bool_nn = true),
                                            decltype(bar.bool_nn = false)>);
  static_assert(cannot_call_insert_set_with<decltype(bar.bool_nn = true),
                                            decltype(dynamic(false, bar.bool_nn = false))>);

  // insert_into(table).set(<assignments from more than one table>) is
  // inconsistent and cannot be constructed.
  static_assert(cannot_call_insert_set_with<decltype(foo.int_n = 7),
                                            decltype(bar.bool_nn = false)>);
  static_assert(cannot_call_insert_set_with<decltype(dynamic(false, foo.int_n = 7)),
                                            decltype(bar.bool_nn = false)>);

  // insert_into(table).set(<not all required columns>) is inconsistent but can
  // be constructed (check can only run later)
  {
    auto i = insert_into(bar).set(bar.int_n = sqlpp::default_value);
    using I = decltype(i);
    expect_basic_consistency_fails<
        I,
        "insert: required column 'bool_nn' is missing">();
  }
  {
    auto i = insert_into(bar).set(dynamic(true, bar.int_n = sqlpp::default_value));
    using I = decltype(i);
    expect_basic_consistency_fails<
        I,
        "insert: required column 'bool_nn' is missing">();
  }

  // insert_into(table).set(<dynamic required columns>) is also inconsistent but
  // can be constructed (check can only run later)
  {
    auto i = insert_into(bar).set(dynamic(true, bar.bool_nn = true));
    using I = decltype(i);
    expect_basic_consistency_fails<
        I,
        "insert: required column 'bool_nn' is missing">();
  }

  // -------------------------
  // insert_into(tab).columns(...)
  // -------------------------

  // insert_into(table).columns(<non arguments>) is inconsistent and cannot be
  // constructed.
  static_assert(cannot_call_insert_columns_with<>);

  // insert_into(table).columns(<arguments including non-columns>) is
  // inconsistent and cannot be constructed.
  static_assert(can_call_insert_columns_with<decltype(bar.int_n)>);
  static_assert(cannot_call_insert_columns_with<decltype(bar.int_n = 7)>);

  // insert_into(table).columns(dynamic arguments) cannot be constructed.
  static_assert(cannot_call_insert_columns_with<decltype(dynamic(true, bar.int_n))>);

  // insert_into(table).columns(duplicate columns>) is
  // inconsistent and cannot be constructed.
  static_assert(
      cannot_call_insert_columns_with<decltype(bar.bool_nn), decltype(bar.int_n),
                                      decltype(bar.bool_nn)>);
  static_assert(
      cannot_call_insert_columns_with<decltype(bar.bool_nn), decltype(bar.int_n),
                                      decltype(dynamic(false, bar.bool_nn))>);

  // insert_into(table).columns(<columns from more than one table>) is
  // inconsistent and cannot be constructed.
  static_assert(
      cannot_call_insert_columns_with<decltype(bar.bool_nn), decltype(foo.int_n)>);
  static_assert(
      cannot_call_insert_columns_with<decltype(dynamic(false, bar.bool_nn)),
                                      decltype(foo.int_n)>);

  // insert_into(table).columns(<not all required columns>) is inconsistent but
  // can be constructed (check can only run later)
  {
    auto i = insert_into(bar).columns(bar.int_n);
    using I = decltype(i);
      expect_basic_consistency_fails<
        I,
        "insert: required column 'bool_nn' is missing">();
  }

  // -------------------------
  // insert_into(tab).columns(...).add_value(...)
  // -------------------------
  {
    auto i = insert_into(bar).columns(bar.int_n, bar.bool_nn);
    using I = decltype(i);

    // OK, correct assignments
    static_assert(can_call_add_values_with<I, decltype(bar.int_n = 7),
                                           decltype(bar.bool_nn = true)>);
    static_assert(
        can_call_add_values_with<I, decltype(bar.int_n = sqlpp::default_value),
                                 decltype(bar.bool_nn = true)>);

    static_assert(
        can_call_add_values_with<I, decltype(dynamic(true, bar.int_n = 42)),
                                 decltype(bar.bool_nn = true)>);

    // Not OK, missing assignment
    static_assert(cannot_call_add_values_with<I, decltype(bar.bool_nn = true)>);
    static_assert(cannot_call_add_values_with<I, decltype(bar.int_n = 7)>);
    static_assert(
        cannot_call_add_values_with<I,
                                    decltype(bar.int_n = sqlpp::default_value)>);

    // Not OK, cannot assign expressions
    static_assert(
        cannot_call_add_values_with<I,
                                    decltype(bar.int_n = sqlpp::default_value),
                                    decltype(bar.bool_nn = not bar.bool_nn)>);

    // Not OK, cannot assign parameters
    static_assert(
        cannot_call_add_values_with<I, decltype(bar.int_n = parameter(bar.int_n)),
                                    decltype(bar.bool_nn = true)>);
    static_assert(cannot_call_add_values_with<
                  I, decltype(bar.int_n = sqlpp::default_value),
                  decltype(bar.bool_nn = parameter(bar.bool_nn))>);

    // Not OK, cannot assign named values
    static_assert(
        cannot_call_add_values_with<I,
                                    decltype(bar.int_n = sqlpp::default_value),
                                    decltype(bar.bool_nn = bar.bool_nn)>);
  }

  // In custom queries, it would be possible to have unknown tables, too.
  {
    auto i = insert_columns(bar.int_n);
    using I = decltype(i);
    expect_basic_consistency_fails<
        I,
        "The insert-columns-clause requires table tab_bar which is not known "
        "in the statement">();
  }

  {
    auto i = from(dynamic(true, bar)) << insert_columns(bar.int_n);
    using I = decltype(i);
    expect_basic_consistency_fails<
        I,
        "The insert-columns-clause statically requires table tab_bar which is "
        "only known dynamically in the statement">();
  }

  {
    auto i = from(dynamic(true, bar)) << insert_set(bar.int_n = 7);
    using I = decltype(i);
    expect_basic_consistency_fails<
        I,
        "The insert-set-clause statically requires table tab_bar which is only "
        "known dynamically in the statement">();
  }

  {
    auto i = insert_set(bar.int_n = 7);
    using I = decltype(i);
    expect_basic_consistency_fails<
        I,
        "The insert-set-clause requires table tab_bar which is not known "
        "in the statement">();
  }

}
