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

#include <sqlpp23/tests/sqlite3/all.h>

int main() {
  auto db = sqlpp::sqlite3::make_test_connection();
  auto ctx = sqlpp::sqlite3::context_t{&db};
  using CTX = decltype(ctx);

  const auto foo = test::TabFoo{};

  // sqlite3 does not support ANY
  {
    auto a = any(select(foo.id).from(foo));
    auto s = select(all_of(foo)).from(foo).where(foo.intN == a);
    auto c = sqlpp::cte(sqlpp::alias::a).as(s);
    auto w = with(c);

    static_assert(std::is_same<decltype(check_compatibility<CTX>(a)),
                               sqlpp::sqlite3::assert_no_any_t>::value);
    // Just checking if the constraint is passed through as it should
    static_assert(std::is_same<decltype(check_compatibility<CTX>(s)),
                               sqlpp::sqlite3::assert_no_any_t>::value);
    static_assert(std::is_same<decltype(check_compatibility<CTX>(c)),
                               sqlpp::sqlite3::assert_no_any_t>::value);
    static_assert(std::is_same<decltype(check_compatibility<CTX>(w)),
                               sqlpp::sqlite3::assert_no_any_t>::value);
  }
}
