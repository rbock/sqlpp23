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

#include <sqlpp23/tests/core/constraints_helpers.h>

#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/core/clause/returning.h>
#include <sqlpp23/tests/core/tables.h>

namespace {
SQLPP_CREATE_NAME_TAG(something);

template <typename... Expressions>
concept can_call_returning_with_standalone =
    requires(Expressions... expressions) { sqlpp::returning(expressions...); };
template <typename... Expressions>
concept can_call_returning_with_in_statement =
    requires(Expressions... expressions) {
      sqlpp::statement_t<sqlpp::no_returning_t>{}.returning(expressions...);
    };

template <typename... Expressions>
concept can_call_returning_with =
    can_call_returning_with_standalone<Expressions...> and
    can_call_returning_with_in_statement<Expressions...>;

template <typename... Expressions>
concept cannot_call_returning_with =
    not(can_call_returning_with_standalone<Expressions...> or
        can_call_returning_with_in_statement<Expressions...>);
}  // namespace

int main() {
  const auto maybe = true;
  const auto foo = test::TabFoo{};
  const auto bar = test::TabBar{};

  using sqlpp::returning;

  // OK
  returning(foo.id);
  returning(foo.id, foo.textNnD);
  returning(all_of(foo));
  returning(foo.id, bar.id);
  returning(all_of(foo), bar.id.as(something));

  // -------------------------
  // returning() can be constructed, but is inconsistent since no
  // columns are selected.
  // -------------------------
  {
    static_assert(cannot_call_returning_with<>, "");
  }

  // -------------------------
  // returning(<unnamed>) can be constructed, but is inconsistent
  // columns require a name.
  // -------------------------
  {
    static_assert(cannot_call_returning_with<decltype(7)>);
    static_assert(
        cannot_call_returning_with<decltype(sqlpp::dynamic(maybe, 7))>);
    static_assert(
        cannot_call_returning_with<decltype(all_of(bar)), decltype(7)>);
  }

  // -------------------------
  // returning is not required
  // -------------------------
  {
    using I = sqlpp::statement_t<sqlpp::no_returning_t>;

    static_assert(std::is_same<sqlpp::statement_consistency_check_t<I>,
                               sqlpp::consistent_t>::value,
                  "");
    static_assert(std::is_same<sqlpp::statement_prepare_check_t<I>,
                               sqlpp::consistent_t>::value,
                  "");
  }

  // -------------------------
  // insert(...).returning(<aggregate functions>)
  // -------------------------
  {
    auto i = sqlpp::insert_into(foo).default_values() << returning(
        max(foo.id).as(something));
    using I = decltype(i);

    static_assert(
        std::is_same<
            sqlpp::statement_consistency_check_t<I>,
            sqlpp::
                assert_returning_columns_contain_no_aggregates_t>::value,
        "");
  }

  // -------------------------
  // insert(...).returning(<unknown_tables>)
  // -------------------------
  {
    auto i =
        sqlpp::insert_into(foo).default_values() << returning(bar.id);
    using I = decltype(i);

    static_assert(std::is_same<sqlpp::statement_consistency_check_t<I>,
                               sqlpp::consistent_t>::value,
                  "");
    static_assert(
        std::is_same<
            sqlpp::statement_prepare_check_t<I>,
            sqlpp::
                assert_no_unknown_tables_in_returning_columns_t>::value,
        "");
  }
}
