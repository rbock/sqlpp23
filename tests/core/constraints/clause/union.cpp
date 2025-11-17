/*
 * Copyright (c) 2025, Roland Bock
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

#include <sqlpp23/tests/core/all.h>

namespace {
SQLPP_CREATE_NAME_TAG(something);

template <typename Lhs, typename Rhs>
concept can_call_union_all_with_standalone =
    requires(Lhs lhs, Rhs rhs) { sqlpp::union_all(lhs, rhs); };
template <typename Lhs, typename Rhs>
concept can_call_union_all_with_in_statement =
    requires(Lhs lhs, Rhs rhs) { lhs.union_all(rhs); };

template <typename Lhs, typename Rhs>
concept can_call_union_all_with =
    can_call_union_all_with_standalone<Lhs, Rhs> and
    can_call_union_all_with_in_statement<Lhs, Rhs>;

template <typename Lhs, typename Rhs>
concept cannot_call_union_all_with =
    not(can_call_union_all_with_standalone<Lhs, Rhs> or
        can_call_union_all_with_in_statement<Lhs, Rhs>);

template <typename Lhs, typename Rhs>
concept can_call_union_distinct_with_standalone =
    requires(Lhs lhs, Rhs rhs) { sqlpp::union_distinct(lhs, rhs); };
template <typename Lhs, typename Rhs>
concept can_call_union_distinct_with_in_statement =
    requires(Lhs lhs, Rhs rhs) { lhs.union_distinct(rhs); };

template <typename Lhs, typename Rhs>
concept can_call_union_distinct_with =
    can_call_union_distinct_with_standalone<Lhs, Rhs> and
    can_call_union_distinct_with_in_statement<Lhs, Rhs>;

template <typename Lhs, typename Rhs>
concept cannot_call_union_distinct_with =
    not(can_call_union_distinct_with_standalone<Lhs, Rhs> or
        can_call_union_distinct_with_in_statement<Lhs, Rhs>);

#define CAN_CALL_ALL_UNIONS_WITH(LHS, RHS)                                  \
  static_assert(can_call_union_all_with<decltype(LHS), decltype(RHS)>, ""); \
  static_assert(can_call_union_distinct_with<decltype(LHS), decltype(RHS)>, "");

#define CANNOT_CALL_ANY_UNION_WITH(LHS, RHS)                                   \
  static_assert(cannot_call_union_all_with<decltype(LHS), decltype(RHS)>, ""); \
  static_assert(cannot_call_union_distinct_with<decltype(LHS), decltype(RHS)>, \
                "");

}  // namespace

int main() {
  const auto maybe = true;
  const auto bar = test::TabBar{};
  const auto foo = test::TabFoo{};

  const auto incomplete_lhs = select(all_of(bar));
  const auto lhs = incomplete_lhs.from(bar);
  const auto incomplete_rhs = select(all_of(bar.as(something)));
  const auto rhs = incomplete_rhs.from(bar.as(something));

  union_distinct(lhs, rhs);
  static_assert(
      can_call_union_all_with_in_statement<decltype(lhs), decltype(rhs)>,
      "");  // OK
  CAN_CALL_ALL_UNIONS_WITH(lhs, rhs);
  CAN_CALL_ALL_UNIONS_WITH(lhs, dynamic(maybe, rhs));

  // Cannot union with non-statement
  CANNOT_CALL_ANY_UNION_WITH(lhs, bar);
  CANNOT_CALL_ANY_UNION_WITH(lhs, bar.id);
  CANNOT_CALL_ANY_UNION_WITH(lhs, all_of(bar));
  CANNOT_CALL_ANY_UNION_WITH(bar, rhs);
  CANNOT_CALL_ANY_UNION_WITH(bar.id, rhs);
  CANNOT_CALL_ANY_UNION_WITH(all_of(bar), rhs);

  // UNION requires statements with result row
  {
    const auto bad_custom_lhs = sqlpp::statement_t<sqlpp::no_union_t>{};
    const auto bad_custom_rhs = sqlpp::statement_t<>{};
    CANNOT_CALL_ANY_UNION_WITH(bad_custom_lhs, rhs);
    CANNOT_CALL_ANY_UNION_WITH(lhs, bad_custom_rhs);
  }

  // UNION requires statements with same result row (single column)
  {
    auto s_foo_int = select(foo.id).from(foo);
    auto s_foo_int_n = select(foo.intN).from(foo);
    auto s_value_id = select(sqlpp::value(7).as(foo.id)).from(foo);
    auto s_value_oid =
        select(sqlpp::value(7).as(something)).from(foo);
    // Different value type
    static_assert(
        not std::is_same<sqlpp::data_type_of_t<decltype(foo.id)>,
                         sqlpp::data_type_of_t<decltype(foo.intN)>>::value,
        "");
    CANNOT_CALL_ANY_UNION_WITH(s_foo_int, s_foo_int_n);
    // Different name
    CANNOT_CALL_ANY_UNION_WITH(s_value_id, s_value_oid);
    CANNOT_CALL_ANY_UNION_WITH(s_foo_int, dynamic(maybe, s_foo_int_n));
    // Different name
    CANNOT_CALL_ANY_UNION_WITH(s_value_id, dynamic(maybe, s_value_oid));
  }

  // UNION requires statements with same result row (more than one column)
  {
    auto s_foo_int = select(foo.textNnD, foo.id).from(foo);
    auto s_foo_int_n = select(foo.textNnD, foo.intN).from(foo);
    auto s_value_id = select(foo.textNnD, sqlpp::value(7).as(foo.id)).from(foo);
    auto s_value_oid =
        select(foo.textNnD, sqlpp::value(7).as(something)).from(foo);
    // Different value type
    static_assert(
        not std::is_same<sqlpp::data_type_of_t<decltype(foo.id)>,
                         sqlpp::data_type_of_t<decltype(foo.intN)>>::value,
        "");
    CANNOT_CALL_ANY_UNION_WITH(s_foo_int, s_foo_int_n);
    // Different name
    CANNOT_CALL_ANY_UNION_WITH(s_value_id, s_value_oid);
    CANNOT_CALL_ANY_UNION_WITH(s_foo_int, dynamic(maybe, s_foo_int_n));
    // Different name
    CANNOT_CALL_ANY_UNION_WITH(s_value_id, dynamic(maybe, s_value_oid));
  }

  // UNION arguments must not contain for_update.
  {
    auto s_foo = select(foo.id).from(foo);
    auto s_bar = select(bar.id).from(bar);

    // Do not allow for_update in the LHS expression.
    CANNOT_CALL_ANY_UNION_WITH(s_foo.for_update(), s_foo);
    CANNOT_CALL_ANY_UNION_WITH(s_foo.for_update(), s_bar);

    // Do not allow for_update in the RHS expression.
    CANNOT_CALL_ANY_UNION_WITH(s_foo, s_bar.for_update());
    CANNOT_CALL_ANY_UNION_WITH(s_bar, s_bar.for_update());

    // Do not allow for_update in both expressions.
    CANNOT_CALL_ANY_UNION_WITH(s_bar.for_update(),
                               s_bar.for_update());
    CANNOT_CALL_ANY_UNION_WITH(s_bar.for_update(),
                               s_bar.for_update());
  }

  // UNION arguments must not contain order_by.
  {
    auto s_foo = select(foo.id).from(foo);
    auto s_bar = select(bar.id).from(bar);

    // Do not allow order_by in the LHS expression.
    CANNOT_CALL_ANY_UNION_WITH(s_foo.order_by(foo.id.asc()), s_foo);
    CANNOT_CALL_ANY_UNION_WITH(s_foo.order_by(foo.id.asc()), s_bar);

    // Do not allow order_by in the RHS expression.
    CANNOT_CALL_ANY_UNION_WITH(s_foo, s_bar.order_by(foo.id.asc()));
    CANNOT_CALL_ANY_UNION_WITH(s_bar, s_bar.order_by(foo.id.asc()));

    // Do not allow order_by in both expressions.
    CANNOT_CALL_ANY_UNION_WITH(s_bar.order_by(foo.id.asc()),
                               s_bar.order_by(foo.id.asc()));
    CANNOT_CALL_ANY_UNION_WITH(s_bar.order_by(foo.id.asc()),
                               s_bar.order_by(foo.id.asc()));
  }

  // UNION arguments must not contain union_order_by.
  {
    auto s_foo = select(foo.id).from(foo).union_all(select(foo.id).from(foo));
    auto s_bar = select(bar.id).from(bar);

    // Do not allow order_by in the LHS expression.
    CANNOT_CALL_ANY_UNION_WITH(s_foo.order_by(foo.id.asc()), s_foo);
    CANNOT_CALL_ANY_UNION_WITH(s_foo.order_by(foo.id.asc()), s_bar);

    // Do not allow order_by in the RHS expression.
    CANNOT_CALL_ANY_UNION_WITH(s_foo, s_bar.order_by(foo.id.asc()));
    CANNOT_CALL_ANY_UNION_WITH(s_bar, s_bar.order_by(foo.id.asc()));

    // Do not allow order_by in both expressions.
    CANNOT_CALL_ANY_UNION_WITH(s_bar.order_by(foo.id.asc()),
                               s_bar.order_by(foo.id.asc()));
    CANNOT_CALL_ANY_UNION_WITH(s_bar.order_by(foo.id.asc()),
                               s_bar.order_by(foo.id.asc()));
  }

  // UNION arguments must not contain order_by.
  {
    auto s_foo = select(foo.id).from(foo);
    auto s_bar = select(bar.id).from(bar);

    // Do not allow order_by in the LHS expression.
    CANNOT_CALL_ANY_UNION_WITH(s_foo.order_by(foo.id.asc()), s_foo);
    CANNOT_CALL_ANY_UNION_WITH(s_foo.order_by(foo.id.asc()), s_bar);

    // Do not allow order_by in the RHS expression.
    CANNOT_CALL_ANY_UNION_WITH(s_foo, s_bar.order_by(foo.id.asc()));
    CANNOT_CALL_ANY_UNION_WITH(s_bar, s_bar.order_by(foo.id.asc()));

    // Do not allow order_by in both expressions.
    CANNOT_CALL_ANY_UNION_WITH(s_bar.order_by(foo.id.asc()),
                               s_bar.order_by(foo.id.asc()));
    CANNOT_CALL_ANY_UNION_WITH(s_bar.order_by(foo.id.asc()),
                               s_bar.order_by(foo.id.asc()));
  }

  // UNION arguments must not contain limit.
  {
    auto s_foo = select(foo.id).from(foo);
    auto s_bar = select(bar.id).from(bar);

    // Do not allow limit in the LHS expression.
    CANNOT_CALL_ANY_UNION_WITH(s_foo.limit(7), s_foo);
    CANNOT_CALL_ANY_UNION_WITH(s_foo.limit(7), s_bar);

    // Do not allow limit in the RHS expression.
    CANNOT_CALL_ANY_UNION_WITH(s_foo, s_bar.limit(7));
    CANNOT_CALL_ANY_UNION_WITH(s_bar, s_bar.limit(7));

    // Do not allow limit in both expressions.
    CANNOT_CALL_ANY_UNION_WITH(s_bar.limit(7),
                               s_bar.limit(7));
    CANNOT_CALL_ANY_UNION_WITH(s_bar.limit(7),
                               s_bar.limit(7));
  }

  // UNION arguments must not contain offset.
  {
    auto s_foo = select(foo.id).from(foo);
    auto s_bar = select(bar.id).from(foo);

    // Do not allow offset in the LHS expression.
    CANNOT_CALL_ANY_UNION_WITH(s_foo.offset(42), s_foo);
    CANNOT_CALL_ANY_UNION_WITH(s_foo.offset(42), s_bar);

    // Do not allow offset in the RHS expression.
    CANNOT_CALL_ANY_UNION_WITH(s_foo, s_bar.offset(42));
    CANNOT_CALL_ANY_UNION_WITH(s_bar, s_bar.offset(42));

    // Do not allow offset in both expressions.
    CANNOT_CALL_ANY_UNION_WITH(s_bar.offset(42),
                               s_bar.offset(42));
    CANNOT_CALL_ANY_UNION_WITH(s_bar.offset(42),
                               s_bar.offset(42));
  }

  // UNION requires preparable statements
  {
    auto u = union_all(lhs, incomplete_rhs);
    using U = decltype(u);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<U>,
                               sqlpp::consistent_t>::value);
    static_assert(
        std::is_same<
            sqlpp::statement_prepare_check_t<U>,
            sqlpp::assert_no_unknown_tables_in_selected_columns_t>::value);
  }
  {
    auto u = union_all(incomplete_lhs, rhs);
    using U = decltype(u);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<U>,
                               sqlpp::consistent_t>::value);
    static_assert(
        std::is_same<
            sqlpp::statement_prepare_check_t<U>,
            sqlpp::assert_no_unknown_tables_in_selected_columns_t>::value);
  }

  // union can be used as sub query referring to tables of the enclosing query
  {
    auto u = select(value(union_all(select(foo.id), select(bar.id))).as(something));
    using U = decltype(u);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<U>,
                               sqlpp::consistent_t>::value);
    static_assert(
        std::is_same<
            sqlpp::statement_prepare_check_t<U>,
            sqlpp::assert_no_unknown_tables_in_selected_columns_t>::value);
  }
}
