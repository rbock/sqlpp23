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

#include <sqlpp23/mysql/mysql.h>
#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/tests/core/tables.h>
#include <sqlpp23/tests/mysql/make_test_connection.h>

int main() {
  auto db = sqlpp::mysql::make_test_connection();
  auto ctx = sqlpp::mysql::context_t{&db};
  using CTX = decltype(ctx);

  const auto foo = test::TabFoo{};
  const auto bar = test::TabBar{};

  // OK
  std::ignore = to_sql_string(ctx, foo.join(bar).on(true));

  // MySQL does not support full outer join.
  {
    auto j = foo.full_outer_join(bar).on(foo.id == bar.id);
    auto f = from(j);
    auto s = select(foo.id, bar.intN) << f;
    auto w = with(cte(sqlpp::alias::a).as(s));

    static_assert(
        std::is_same<decltype(check_compatibility<CTX>(j)),
                     sqlpp::mysql::assert_no_full_outer_join_t>::value);
    static_assert(
        std::is_same<decltype(check_compatibility<CTX>(f)),
                     sqlpp::mysql::assert_no_full_outer_join_t>::value);
    static_assert(
        std::is_same<decltype(check_compatibility<CTX>(s)),
                     sqlpp::mysql::assert_no_full_outer_join_t>::value);
    static_assert(
        std::is_same<decltype(check_compatibility<CTX>(w)),
                     sqlpp::mysql::assert_no_full_outer_join_t>::value);
  }
}
