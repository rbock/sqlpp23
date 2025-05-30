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
SQLPP_CREATE_NAME_TAG(something);

template <typename... Expressions>
concept can_call_group_by_with_standalone =
    requires(Expressions... expressions) { sqlpp::group_by(expressions...); };
template <typename... Expressions>
concept can_call_group_by_with_in_statement =
    requires(Expressions... expressions) {
      sqlpp::statement_t<sqlpp::no_group_by_t>{}.group_by(expressions...);
    };

template <typename... Expressions>
concept can_call_group_by_with =
    can_call_group_by_with_standalone<Expressions...> and
    can_call_group_by_with_in_statement<Expressions...>;

template <typename... Expressions>
concept cannot_call_group_by_with =
    not(can_call_group_by_with_standalone<Expressions...> or
        can_call_group_by_with_in_statement<Expressions...>);
}  // namespace

int main() {
  const auto maybe = true;
  const auto foo = test::TabFoo{};
  const auto bar = test::TabBar{};

  // group_by(<non arguments>) is inconsistent and cannot be constructed.
  static_assert(cannot_call_group_by_with<>);

  // group_by(<arguments with no value>) cannot be called.
  static_assert(can_call_group_by_with<decltype(bar.boolNn)>);
  static_assert(can_call_group_by_with<decltype(dynamic(maybe, bar.boolNn))>);
  static_assert(can_call_group_by_with<decltype(bar.id + 7)>);
  static_assert(can_call_group_by_with<decltype(7), decltype(bar.boolNn)>);

  static_assert(
      cannot_call_group_by_with<decltype(bar.intN = 7), decltype(bar.boolNn)>,
      "not value: assignment");
  static_assert(
      cannot_call_group_by_with<decltype(all_of(bar)), decltype(bar.boolNn)>,
      "not value: tuple");

  // group_by(<containing aggregate functions>) is inconsistent and cannot be
  // constructed.
  static_assert(cannot_call_group_by_with<decltype(max(foo.id))>);

  // group_by is not required
  {
    auto s = sqlpp::statement_t<sqlpp::no_group_by_t>{};
    using S = decltype(s);
    static_assert(std::is_same<sqlpp::statement_prepare_check_t<S>,
                               sqlpp::consistent_t>::value,
                  "");
  }

  // group_by must not require unknown tables for prepare/run
  {
    auto s = select(foo.id).from(foo).group_by(foo.id);
    using S = decltype(s);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<S>,
                               sqlpp::consistent_t>::value,
                  "");
    static_assert(std::is_same<sqlpp::statement_prepare_check_t<S>,
                               sqlpp::consistent_t>::value,
                  "");
  }

  {
    auto s = select(foo.id).from(foo).group_by(foo.id, bar.id);
    using S = decltype(s);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<S>,
                               sqlpp::consistent_t>::value,
                  "");
    static_assert(
        std::is_same<sqlpp::statement_prepare_check_t<S>,
                     sqlpp::assert_no_unknown_tables_in_group_by_t>::value,
        "");
  }

  // `group_by` using unknown table
  {
    auto s = select(max(foo.id).as(something)).from(foo).group_by(bar.id);
    using S = decltype(s);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<S>,
                               sqlpp::consistent_t>::value,
                  "");
    static_assert(
        std::is_same<sqlpp::statement_prepare_check_t<S>,
                     sqlpp::assert_no_unknown_tables_in_group_by_t>::value,
        "");
  }

  // `group_by` statically using dynamic table
  {
    auto s = select(max(foo.id).as(something))
                 .from(foo.cross_join(dynamic(maybe, bar)))
                 .group_by(bar.id);
    using S = decltype(s);
    static_assert(
        std::is_same<
            sqlpp::statement_consistency_check_t<S>,
            sqlpp::assert_no_unknown_static_tables_in_group_by_t>::value,
        "");
    static_assert(
        std::is_same<
            sqlpp::statement_prepare_check_t<S>,
            sqlpp::assert_no_unknown_static_tables_in_group_by_t>::value,
        "");
  }
}
