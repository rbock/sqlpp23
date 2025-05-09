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

#include <sqlpp23/core/clause/on_conflict.h>
#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/tests/core/tables.h>

namespace {
SQLPP_CREATE_NAME_TAG(something);

// Test on_conflict
template <typename... Expressions>
concept can_call_on_conflict_with_standalone = requires(
    Expressions... expressions) { sqlpp::on_conflict(expressions...); };
template <typename... Expressions>
concept can_call_on_conflict_with_in_statement =
    requires(Expressions... expressions) {
      sqlpp::statement_t<sqlpp::no_on_conflict_t>{}.on_conflict(expressions...);
    };

template <typename... Expressions>
concept can_call_on_conflict_with =
    can_call_on_conflict_with_standalone<Expressions...> and
    can_call_on_conflict_with_in_statement<Expressions...>;

template <typename... Expressions>
concept cannot_call_on_conflict_with =
    not(can_call_on_conflict_with_standalone<Expressions...> or
        can_call_on_conflict_with_in_statement<Expressions...>);

// Test do_update
template <typename Lhs, typename... Expressions>
concept can_call_do_update_with =
    requires(Lhs lhs, Expressions... expressions) {
      lhs.do_update(expressions...);
    };

template <typename Lhs, typename... Expressions>
concept cannot_call_do_update_with =
    not(can_call_do_update_with<Lhs, Expressions...>);

// Test where
template <typename Lhs, typename... Expressions>
concept can_call_where_with = requires(Lhs lhs, Expressions... expressions) {
  lhs.where(expressions...);
};

template <typename Lhs, typename... Expressions>
concept cannot_call_where_with = not(can_call_where_with<Lhs, Expressions...>);

}  // namespace

int main() {
  const auto maybe = true;
  const auto foo = test::TabFoo{};
  const auto bar = test::TabBar{};

  using sqlpp::on_conflict;

  // OK
  on_conflict();
  on_conflict(foo.id);
  on_conflict(foo.id, foo.textNnD);
  on_conflict(foo.id, bar.id);
  static_assert(can_call_on_conflict_with<>, "");
  static_assert(can_call_on_conflict_with<decltype(foo.id)>, "");

  // -------------------------
  // on_conflict(<non-column>) cannot be constructed.
  // -------------------------
  {
    static_assert(cannot_call_on_conflict_with<decltype(all_of(foo))>);
    static_assert(cannot_call_on_conflict_with<decltype(bar.id.as(something))>,
                  "");
  }

  // do_update requires a conflict target
  {
    auto insert = sqlpp::insert_into(foo).default_values() << on_conflict();
    static_assert(
        cannot_call_do_update_with<decltype(insert), decltype(foo.intN = 7)>);
  }

  // do_update requires assignments as arguments
  {
    auto insert = sqlpp::insert_into(foo).default_values()
                  << on_conflict(foo.id);
    static_assert(
        can_call_do_update_with<decltype(insert), decltype(foo.intN = 7)>);
    static_assert(
        can_call_do_update_with<decltype(insert),
                                decltype(dynamic(maybe, foo.intN = 7))>,
        "");
    static_assert(
        cannot_call_do_update_with<decltype(insert), decltype(foo.intN == 7)>,
        "");

    using I = decltype(insert);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<I>,
                               sqlpp::assert_on_conflict_action_t>::value,
                  "");
    static_assert(std::is_same<sqlpp::statement_prepare_check_t<I>,
                               sqlpp::assert_on_conflict_action_t>::value,
                  "");
  }

  // More do_update requirements
  {
    const auto insert = sqlpp::insert_into(foo).default_values()
                        << on_conflict(foo.id);
    // Need at least one assignment
    static_assert(cannot_call_do_update_with<decltype(insert)>);

    // Assignment columns need to be unique
    static_assert(
        cannot_call_do_update_with<decltype(insert), decltype(foo.intN = 5),
                                   decltype(foo.intN = 19)>);

    // Assignment columns need to be unique
    static_assert(
        cannot_call_do_update_with<decltype(insert), decltype(foo.intN = 5),
                                   decltype(bar.boolNn = false)>);
  }

  // do_update can be qualified with `where` being called with a single boolean
  // expression
  {
    auto insert = sqlpp::insert_into(foo).default_values()
                  << on_conflict(foo.id).do_update(foo.intN = 7);

    static_assert(can_call_where_with<decltype(insert), decltype(foo.id == 7)>,
                  "");
    static_assert(can_call_where_with<decltype(insert),
                                      decltype(dynamic(maybe, foo.id == 7))>,
                  "");
    static_assert(
        cannot_call_where_with<decltype(insert), decltype(foo.intN = 7)>, "");
    static_assert(
        cannot_call_where_with<decltype(insert), decltype(foo.intN = 7),
                               decltype(true)>,
        "");
  }

  // do_update.where in-function checks
  {
    auto insert = sqlpp::insert_into(foo).default_values()
                  << on_conflict(foo.id).do_update(foo.intN = 7);
    static_assert(cannot_call_where_with<decltype(insert), decltype(max(foo.id) > 3)>);
  }

  // -----------------------------------------
  // bad table checks
  // -----------------------------------------
  {
    auto insert = sqlpp::insert_into(foo).default_values()
                  << on_conflict(bar.id).do_update(foo.intN = 7);
    using I = decltype(insert);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<I>,
                               sqlpp::consistent_t>::value,
                  "");
    static_assert(
        std::is_same<
            sqlpp::statement_prepare_check_t<I>,
            sqlpp::assert_no_unknown_tables_in_on_conflict_do_update_t>::value,
        "");
  }

  {
    auto insert = sqlpp::insert_into(foo).default_values()
                  << on_conflict(foo.id).do_update(bar.intN = 7);
    using I = decltype(insert);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<I>,
                               sqlpp::consistent_t>::value,
                  "");
    static_assert(
        std::is_same<
            sqlpp::statement_prepare_check_t<I>,
            sqlpp::assert_no_unknown_tables_in_on_conflict_do_update_t>::value,
        "");
  }

  {
    auto insert =
        sqlpp::insert_into(foo).default_values()
        << on_conflict(foo.id).do_update(foo.intN = 7).where(bar.id > 8);
    using I = decltype(insert);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<I>,
                               sqlpp::consistent_t>::value,
                  "");
    static_assert(
        std::is_same<
            sqlpp::statement_prepare_check_t<I>,
            sqlpp::assert_no_unknown_tables_in_on_conflict_do_update_t>::value,
        "");
  }

  // Dynamically provided tables are not a thing in `insert_into`. Constructing
  // a nonsense custom query.
  //
  // Don't do this home!
  {
    auto nonsense =
        from(dynamic(maybe, foo))
        << on_conflict(foo.id).do_update(foo.intN = 7).where(foo.id > 8);
    using I = decltype(nonsense);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<I>,
                               sqlpp::consistent_t>::value,
                  "");
    static_assert(
        std::is_same<
            sqlpp::statement_prepare_check_t<I>,
            sqlpp::assert_no_unknown_static_tables_in_on_conflict_do_update_t>::
            value,
        "");
  }
}
