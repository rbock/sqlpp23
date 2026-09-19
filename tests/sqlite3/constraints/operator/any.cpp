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

#include <sqlpp26/tests/sqlite3/all.h>

int main() {
  auto db = sqlpp::sqlite3::make_test_connection();
  auto ctx = sqlpp::sqlite3::context_t{&db};
  using CTX = decltype(ctx);

  const auto foo = test::tab_foo{};

  // sqlite3 does not support ANY
  {
    auto a = any(select(foo.id).from(foo));
    auto s = select(all_of(foo)).from(foo).where(foo.int_n == a);
    auto c = sqlpp::cte<"a">().as(s);
    auto w = with(c);

    expect_compatibility_fails<CTX, decltype(a), "Sqlite3: No support for any()">();
    // Just checking if the constraint is passed through as it should
    expect_compatibility_fails<CTX, decltype(s), "Sqlite3: No support for any()">();
    expect_compatibility_fails<CTX, decltype(c), "Sqlite3: No support for any()">();
    expect_compatibility_fails<CTX, decltype(w), "Sqlite3: No support for any()">();
  }
}
