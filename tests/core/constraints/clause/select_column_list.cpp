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
#include "sqlpp26/core/detail/type_set.h"

namespace {
template <typename... Expressions>
concept can_call_select_columns_with_standalone = requires(
    Expressions... expressions) { sqlpp::select_columns(expressions...); };
template <typename... Expressions>
concept can_call_select_columns_with_in_statement = requires(
    Expressions... expressions) {
  sqlpp::statement_t<sqlpp::no_select_column_list_t>{}.columns(expressions...);
};

template <typename... Expressions>
concept can_call_select_columns_with =
    can_call_select_columns_with_standalone<Expressions...> and
    can_call_select_columns_with_in_statement<Expressions...>;

template <typename... Expressions>
concept cannot_call_select_columns_with =
    not(can_call_select_columns_with_standalone<Expressions...> or
        can_call_select_columns_with_in_statement<Expressions...>);
}  // namespace

namespace test {
SQLPP_CREATE_NAME_TAG(max_id);
}

int main() {
  const auto foo = test::tab_foo{};
  const auto bar = test::tab_bar{};

  // Confirming the required columns of tab_bar.
  static_assert(sqlpp::required_insert_columns_of<test::tab_bar>::func() ==
                             sqlpp::detail::make_type_info_set<sqlpp::column<
                                 test::tab_bar, 2>>());

  // -------------------------
  // select() can be constructed, but is inconsistent since not columns are
  // selected.
  // -------------------------
  {
    auto s = sqlpp::select();
    using S = decltype(s);
    expect_basic_consistency_fails<
        S,
        "selecting columns required">();
  }

  // -------------------------
  // select().columns(...)
  // -------------------------

  // select_columns(<no arguments>) is inconsistent and cannot be constructed.
  static_assert(cannot_call_select_columns_with<>);

  // select_columns(<flags only>) is inconsistent and cannot be constructed.
  static_assert(cannot_call_select_columns_with<sqlpp::all_t>);

  // select_columns(<arguments with no value>) cannot be called.
  static_assert(can_call_select_columns_with<decltype(bar.bool_nn)>,
                "OK, argument a column");
  static_assert(
      can_call_select_columns_with<decltype(dynamic(true, bar.bool_nn))>,
      "OK, argument a column");
  static_assert(cannot_call_select_columns_with<decltype(bar.id == 7)>,
                "not a value: comparison");
  static_assert(cannot_call_select_columns_with<sqlpp::all_t, decltype(bar.id == 7)>,
                "not a value: comparison (the leading flag does not change that)");
  static_assert(cannot_call_select_columns_with<decltype(bar.int_n = 7),
                                                decltype(bar.bool_nn)>,
                "not value: assignment");

  // select_columns(<bad flags>) cannot be called.
  static_assert(can_call_select_columns_with<sqlpp::all_t, decltype(bar.id)>,
                "OK, argument a flag and a column");
  static_assert(can_call_select_columns_with<sqlpp::all_t, decltype(all_of(bar))>,
                "OK, argument a flag and a column");

  static_assert(cannot_call_select_columns_with<decltype(bar.id), sqlpp::all_t>,
                "Not OK, flags must not follow columns");
  static_assert(cannot_call_select_columns_with<decltype(all_of(bar)), sqlpp::all_t>,
                "Not OK, flags must not follow columns");
  static_assert(cannot_call_select_columns_with<sqlpp::union_all_t, decltype(bar.id)>,
                "Not OK, incorrect flag");

  // select_columns(<at least one unnamed column>) is inconsistent and cannot be
  // constructed.
  static_assert(cannot_call_select_columns_with<decltype(sqlpp::value(7))>,
                "each selected column must have a name");
  static_assert(
      cannot_call_select_columns_with<decltype(bar.id), decltype(max(foo.id))>,
      "each selected column must have a name");
  static_assert(cannot_call_select_columns_with<decltype(all_of(bar)),
                                                decltype(max(foo.id))>,
                "each selected column must have a name");

  static_assert(
      cannot_call_select_columns_with<decltype(dynamic(true, sqlpp::value(7)))>,
      "each selected column must have a name");
  static_assert(
      cannot_call_select_columns_with<decltype(bar.id),
                                      decltype(dynamic(true, max(foo.id)))>,
      "each selected column must have a name");
  static_assert(cannot_call_select_columns_with<decltype(dynamic(true, bar.id)),
                                                decltype(max(foo.id))>,
                "each selected column must have a name");
  static_assert(
      cannot_call_select_columns_with<decltype(all_of(bar)),
                                      decltype(dynamic(true, max(foo.id)))>,
      "each selected column must have a name");
  // Note: There is no `dynamic(condition, all_of(table))`

  // select_columns(<selecting table columns without `from`>) can be constructed
  // but is inconsistent.
  {
    auto s = sqlpp::select_columns(bar.id);
    using S = decltype(s);
    expect_basic_consistency_succeeds<S>();
    expect_prepare_consistency_fails<
        S,
        "The select-columns-clause requires table tab_bar which is not known "
        "in the statement">();
  }

  {
    auto s = sqlpp::select_columns(dynamic(true, bar.id));
    using S = decltype(s);
    expect_basic_consistency_succeeds<S>();
    expect_prepare_consistency_fails<
        S,
        "The select-columns-clause requires table tab_bar which is not known "
        "in the statement">();
  }

  // ----------------------------
  // ------- Aggregates ---------
  // ----------------------------
  // select_columns(<mix of aggregate and non-aggregate columns>) can be
  // constructed but is inconsistent.
  {
    auto s = select(foo.id, max(foo.id).as<"test::max_id">()).from(foo);
    using S = decltype(s);
    expect_basic_consistency_fails<
        S,
        "without group_by, selected columns must not be a mix of aggregate and "
        "non-aggregate expressions">();
  }

  {
    auto s = select(foo.id, dynamic(false, max(foo.id).as<"test::max_id">())).from(foo);
    using S = decltype(s);
    expect_basic_consistency_fails<
        S,
        "without group_by, selected columns must not be a mix of aggregate and "
        "non-aggregate expressions">();
  }

  {
    auto s = select(foo.id, (max(foo.id) + 7).as<"test::max_id">()).from(foo);
    using S = decltype(s);

    expect_basic_consistency_fails<
        S,
        "without group_by, selected columns must not be a mix of aggregate and "
        "non-aggregate expressions">();
  }

  {
    auto s = select(foo.id, foo.int_n).from(foo).group_by(foo.int_n);
    using S = decltype(s);

    expect_basic_consistency_fails<
        S,
        "select (with group by) must select aggregates only">();
  }

  {
    auto s =
        select(foo.id, dynamic(true, foo.int_n)).from(foo).group_by(foo.int_n);
    using S = decltype(s);
    expect_basic_consistency_fails<
        S,
        "select (with group by) must select aggregates only">();
  }

  {
    auto s = select(foo.id, dynamic(true, (foo.int_n + 7).as<"test::max_id">()))
                 .from(foo)
                 .group_by(foo.int_n);
    using S = decltype(s);
    expect_basic_consistency_fails<
        S,
        "select (with group by) must select aggregates only">();
  }

  // Dynamic group by column
  {
    auto s = select(foo.id, dynamic(true, foo.int_n.as<"test::max_id">()))
                 .from(foo)
                 .group_by(foo.id, dynamic(true, foo.int_n));
    using S = decltype(s);
    expect_basic_consistency_succeeds<S>();
  }
  {
    auto s = select(foo.id, foo.int_n)
                 .from(foo)
                 .group_by(foo.id, dynamic(true, foo.int_n));
    using S = decltype(s);
    expect_basic_consistency_fails<
        S,
        "select statically contains aggregates that are only dynamically "
        "defined in group_by">();
  }
  {
    auto s = select(foo.id, (foo.int_n + 7).as<"test::max_id">())
                 .from(foo)
                 .group_by(foo.id, dynamic(true, foo.int_n));
    using S = decltype(s);
    expect_basic_consistency_fails<
        S,
        "select statically contains aggregates that are only dynamically "
        "defined in group_by">();
  }

  // Non-column group by
  {
    const auto c = foo.id + foo.int_n;
    auto s = select(c.as<"something">())
                 .from(foo)
                 .group_by(c);
    using S = decltype(s);
    expect_basic_consistency_succeeds<S>();
  }
  {
    const auto c = foo.id + foo.int_n;
    auto s = select((foo.float_n + c).as<"something">())
                 .from(foo)
                 .group_by(foo.float_n, c);
    using S = decltype(s);

    expect_basic_consistency_succeeds<S>();
  }
  {
    const auto c = foo.id + foo.int_n;
    auto s = select((foo.id + c).as<"something">())
                 .from(foo)
                 .group_by(foo.float_n, c);
    using S = decltype(s);
    expect_basic_consistency_fails<
        S,
        "select (with group by) must select aggregates only">();
  }
  // ----------------------------
  // ------- Join  --------------
  // ----------------------------
  {
    auto s = select(foo.id).from(bar.cross_join(foo));
    using S = decltype(s);

    expect_basic_consistency_succeeds<S>();
  }

  {
    // Fail: Statically required table, but provided dynamically only
    auto s = select(foo.id).from(bar.cross_join(dynamic(true, foo)));
    using S = decltype(s);
    expect_basic_consistency_fails<
        S,
        "The select-columns-clause statically requires table tab_foo which is "
        "only known dynamically in the statement">();
  }
  {
    // Fail: Statically required table, but provided dynamically only
    auto s = select(foo.id).from(dynamic(true, foo));
    using S = decltype(s);
    expect_basic_consistency_fails<
        S,
        "The select-columns-clause statically requires table tab_foo which is "
        "only known dynamically in the statement">();
  }
  {
    // Fail: This is a sub select that statically requires `foo` but provides it
    // dynamically only.
    auto s = select(foo.id, bar.int_n).from(dynamic(true, foo));
    using S = decltype(s);
    expect_basic_consistency_fails<
        S,
        "The select-columns-clause statically requires table tab_foo which is "
        "only known dynamically in the statement">();
  }
  {
    // Fail: foo is statically required in a selected expression, but provided
    // dynamically only.
    auto s = select((foo.id + bar.int_n).as<"something">()).from(dynamic(true, foo));
    using S = decltype(s);
    expect_basic_consistency_fails<
        S,
        "The select-columns-clause statically requires table tab_foo which is "
        "only known dynamically in the statement">();
  }
  {
    // Fail: `bar` is required, but not provided. This can be used as a
    // sub-select (consistency check), but not as a table (prepare check), and
    // it could not be prepared or executed either.
    auto s = select(bar.id).from(foo);
    using S = decltype(s);
    expect_basic_consistency_succeeds<S>();
    expect_prepare_consistency_fails<
        S,
        "The select-columns-clause requires table tab_bar which is not known in the statement">();
  }
}
