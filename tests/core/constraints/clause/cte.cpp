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

#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/tests/core/tables.h>

namespace {
SQLPP_CREATE_NAME_TAG(something);

template <typename Lhs, typename Rhs>
concept can_call_cte_as_with = requires(Lhs lhs, Rhs rhs) { lhs.as(rhs); };

// Returns true if `declval<Lhs>().UNION(declval<Rhs>)` is a valid function
// call.
#define MAKE_CAN_CALL_CTE_UNION_WITH(UNION) \
  template <typename Lhs, typename Rhs>     \
  concept can_call_cte_##UNION##_with =     \
      requires(Lhs lhs, Rhs rhs) { lhs.UNION(rhs); };

MAKE_CAN_CALL_CTE_UNION_WITH(union_all);
MAKE_CAN_CALL_CTE_UNION_WITH(union_distinct);

#define CAN_CALL_ALL_CTE_UNIONS_WITH(LHS, RHS)                             \
  static_assert(can_call_cte_union_all_with<decltype(LHS), decltype(RHS)>, \
                "");                                                       \
  static_assert(                                                           \
      can_call_cte_union_distinct_with<decltype(LHS), decltype(RHS)>, "");

#define CANNOT_CALL_ANY_UNION_WITH(LHS, RHS)                                   \
  static_assert(not can_call_cte_union_all_with<decltype(LHS), decltype(RHS)>, \
                "");                                                           \
  static_assert(                                                               \
      not can_call_cte_union_distinct_with<decltype(LHS), decltype(RHS)>, "");

}  // namespace

int main() {
  const auto maybe = true;
  const auto bar = test::TabBar{};
  const auto foo = test::TabFoo{};

  const auto ref = sqlpp::cte(something);
  using Ref = decltype(ref);
  const auto incomplete_s1 = select(all_of(bar));
  const auto s1 = incomplete_s1.from(bar);
  const auto incomplete_s2 = select(all_of(bar.as(something)));
  const auto s2 = incomplete_s2.from(bar.as(something));

  const auto cte = ref.as(s1);

  // OK
  static_assert(can_call_cte_as_with<decltype(ref), decltype(s1)>, "");
  static_assert(can_call_cte_as_with<decltype(ref), decltype(s2)>, "");

  // No statement
  static_assert(not can_call_cte_as_with<decltype(ref), decltype(foo)>, "");
  static_assert(not can_call_cte_as_with<decltype(ref), decltype(foo.id)>, "");
  static_assert(not can_call_cte_as_with<decltype(ref), decltype(all_of(foo))>,
                "");

  // No statement
  static_assert(
      not can_call_cte_as_with<decltype(ref), decltype(insert_into(foo))>, "");
  static_assert(
      not can_call_cte_as_with<decltype(ref), decltype(sqlpp::statement_t<>{})>,
      "");

  // common table expression must not use unknown tables
  static_assert(not can_call_cte_as_with<Ref, decltype(select(all_of(bar)))>);
  static_assert(not can_call_cte_as_with<Ref, decltype(select(
                                                  all_of(bar.as(something))))>);
  static_assert(
      not can_call_cte_as_with<Ref, decltype(select(foo.id).from(bar))>);

  // Bad self-reference
  static_assert(not can_call_cte_as_with<Ref, decltype(select(cte.id).from(cte))>);

  // common table expression must not use inconsistent selects
  {
    const auto inconsistent_select =
        select(all_of(bar)).from(bar).having(bar.id > 7);
    static_assert(
        std::is_same<decltype(check_basic_consistency(inconsistent_select)),
                     sqlpp::assert_having_all_aggregates_t>::value);
    static_assert(not can_call_cte_as_with<Ref, decltype(inconsistent_select)>);
  }

  // OK
  CAN_CALL_ALL_CTE_UNIONS_WITH(cte, s1);
  CAN_CALL_ALL_CTE_UNIONS_WITH(cte, s2);

  // Cannot union with non-statement
  CANNOT_CALL_ANY_UNION_WITH(cte, bar);
  CANNOT_CALL_ANY_UNION_WITH(cte, bar.id);
  CANNOT_CALL_ANY_UNION_WITH(cte, all_of(bar));
  CANNOT_CALL_ANY_UNION_WITH(cte, cte);

  // CTE UNION requires statements with result row
  {
    const auto bad_rhs = sqlpp::statement_t<>{};
    CANNOT_CALL_ANY_UNION_WITH(cte, bad_rhs);
  }

  // CTE UNION requires no missing tables
  {
    auto bad_rhs = select(all_of(foo));
    CANNOT_CALL_ANY_UNION_WITH(cte, bad_rhs);
  }

  // CTE UNION requires statements with same result row
  {
    auto c_foo_int =
        sqlpp::cte(something).as(select(foo.textNnD, foo.id).from(foo));
    auto s_foo_int_n = select(foo.textNnD, foo.intN).from(foo);
    auto c_value_id = sqlpp::cte(something).as(
        select(foo.textNnD, sqlpp::value(7).as(foo.id)).from(foo));
    auto s_value_oid =
        select(foo.textNnD, sqlpp::value(7).as(something)).from(foo);

    static_assert(
        not std::is_same<sqlpp::data_type_of_t<decltype(foo.id)>,
                         sqlpp::data_type_of_t<decltype(foo.intN)>>::value,
        "");

    // Different data types
    CANNOT_CALL_ANY_UNION_WITH(c_foo_int, s_foo_int_n);
    CANNOT_CALL_ANY_UNION_WITH(c_foo_int, dynamic(maybe, s_foo_int_n));

    // Different name
    CANNOT_CALL_ANY_UNION_WITH(c_value_id, s_value_oid);
    CANNOT_CALL_ANY_UNION_WITH(c_value_id, dynamic(maybe, s_value_oid));
  }
}
