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

// We need to include this here to change the sqlite3 version number for this
// test (if necessary)
#include "sqlpp23/core/type_traits.h"
#ifdef SQLPP_USE_SQLCIPHER
#include <sqlcipher/sqlite3.h>
#else
#include <sqlite3.h>
#endif
#if SQLITE_VERSION_NUMBER >= 3039000
#undef SQLITE_VERSION_NUMBER
#define SQLITE_VERSION_NUMBER 3038999
#endif

#include <sqlpp23/sqlite3/sqlite3.h>
#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/tests/core/tables.h>
#include <sqlpp23/tests/sqlite3/make_test_connection.h>

int main() {
  auto db = sqlpp::sqlite3::make_test_connection();
  auto ctx = sqlpp::sqlite3::context_t{&db};
  using CTX = decltype(ctx);

  const auto foo = test::TabFoo{};
  const auto bar = test::TabBar{};

  // OK
  std::ignore = to_sql_string(ctx, foo.join(bar).on(true));

  // sqlite3 does not support full outer join before 3.39.0
  // See https://www.sqlite.org/changes.html
  {
    auto j = foo.full_outer_join(bar).on(foo.id == bar.id);
    auto f = from(j);
    auto s = select(foo.id, bar.intN) << f;
    auto w = with(sqlpp::cte(sqlpp::alias::a).as(s));

    static_assert(
        std::is_same<decltype(check_compatibility<CTX>(j)),
                     sqlpp::sqlite3::assert_no_full_outer_join_t>::value);
    static_assert(
        std::is_same<decltype(check_compatibility<CTX>(f)),
                     sqlpp::sqlite3::assert_no_full_outer_join_t>::value);
    static_assert(
        std::is_same<decltype(check_compatibility<CTX>(s)),
                     sqlpp::sqlite3::assert_no_full_outer_join_t>::value);
    static_assert(
        std::is_same<decltype(check_compatibility<CTX>(w)),
                     sqlpp::sqlite3::assert_no_full_outer_join_t>::value);
  }

  // sqlite3 does not support right outer join before 3.39.0
  {
    auto j = foo.right_outer_join(bar).on(foo.id == bar.id);
    auto f = from(j);
    auto s = select(foo.id, bar.intN) << f;
    auto w = with(sqlpp::cte(sqlpp::alias::a).as(s));

    static_assert(
        std::is_same<decltype(check_compatibility<CTX>(j)),
                     sqlpp::sqlite3::assert_no_full_outer_join_t>::value);
    static_assert(
        std::is_same<decltype(check_compatibility<CTX>(f)),
                     sqlpp::sqlite3::assert_no_full_outer_join_t>::value);
    static_assert(
        std::is_same<decltype(check_compatibility<CTX>(s)),
                     sqlpp::sqlite3::assert_no_full_outer_join_t>::value);
    static_assert(
        std::is_same<decltype(check_compatibility<CTX>(w)),
                     sqlpp::sqlite3::assert_no_full_outer_join_t>::value);
  }
}
