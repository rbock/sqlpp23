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

#include <sqlpp23/tests/core/all.h>

namespace {
template <typename... Expressions>
concept can_call_union_order_by_with_standalone =
    requires(Expressions... expressions) { sqlpp::union_order_by(expressions...); };
template <typename... Expressions>
concept can_call_union_order_by_with_in_statement =
    requires(Expressions... expressions) {
      sqlpp::statement_t<sqlpp::no_union_order_by_t>{}.order_by(expressions...);
    };

template <typename... Expressions>
concept can_call_union_order_by_with =
    can_call_union_order_by_with_standalone<Expressions...> and
    can_call_union_order_by_with_in_statement<Expressions...>;

template <typename... Expressions>
concept cannot_call_union_order_by_with =
    not(can_call_union_order_by_with_standalone<Expressions...> or
        can_call_union_order_by_with_in_statement<Expressions...>);
}  // namespace

int main() {
  const auto maybe = true;
  const auto foo = test::TabFoo{};
  const auto bar = test::TabBar{};

  // union_order_by(<non arguments>) is inconsistent and cannot be constructed.
  static_assert(cannot_call_union_order_by_with<>);

  // union_order_by(<non-sort-order arguments>) cannot be called.
  static_assert(can_call_union_order_by_with<decltype(bar.boolNn.asc())>,
                "OK, argument a column ascending");
  static_assert(
      can_call_union_order_by_with<decltype(dynamic(maybe, bar.boolNn.desc()))>,
      "OK, argument a dynamic column");
  static_assert(cannot_call_union_order_by_with<decltype((bar.id + 7).asc())>,
                "not a column: bar.id + 7");
  static_assert(cannot_call_union_order_by_with<decltype(bar.id)>,
                "not sort order: column");
  static_assert(cannot_call_union_order_by_with<decltype(7)>,
                "not sort order: integer");
  static_assert(
      cannot_call_union_order_by_with<decltype(bar.intN = 7), decltype(bar.boolNn)>,
      "not sort order: assignment");
  static_assert(
      cannot_call_union_order_by_with<decltype(all_of(bar)), decltype(bar.boolNn)>,
      "not sort order: tuple");

  // union_order_by(<duplicate sort order expressions>) is allowed, see #39
  static_assert(
        can_call_union_order_by_with<decltype(bar.id.asc()), decltype(bar.id.asc())>);
  static_assert(
        can_call_union_order_by_with<decltype(bar.id.asc()), decltype(bar.id.desc())>);
  static_assert(
        can_call_union_order_by_with<decltype(dynamic(false, bar.id.asc())), decltype(bar.id.asc())>);

  // union_order_by is not required
  {
    auto s = sqlpp::statement_t<sqlpp::no_union_order_by_t>{};
    using S = decltype(s);
    static_assert(std::is_same<sqlpp::statement_prepare_check_t<S>,
                               sqlpp::consistent_t>::value,
                  "");
  }

  // union_order_by must not require unknown columns (name / value type)
  // We don't care about tables as we only represent simple, not table-qualified columns.
  {
    auto s = select(foo.id).from(foo).union_all(select(foo.id).from(foo)).order_by(bar.intN.asc());
    using S = decltype(s);

    static_assert(
        std::is_same<
            sqlpp::statement_consistency_check_t<S>,
            sqlpp::assert_no_unknown_columns_in_union_sort_order_t>::value,
        "");
    static_assert(
        std::is_same<
            sqlpp::statement_prepare_check_t<S>,
            sqlpp::assert_no_unknown_columns_in_union_sort_order_t>::value,
        "");
  }

}
