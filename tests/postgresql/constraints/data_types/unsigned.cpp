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

#include <sqlpp26/tests/postgresql/all.h>

int main() {
  auto db = sqlpp::postgresql::make_test_connection();
  auto ctx = sqlpp::postgresql::context_t{&db};
  using CTX = decltype(ctx);

  // OK
  {
    auto ci = cast(std::nullopt, sqlpp::as<sqlpp::boolean>());

    expect_compatibility_succeeds<CTX, decltype(ci)>();
  }

  // Postgresql has no support for unsigned integral
  {
    auto a = 7u;
    auto b = std::optional(7u);
    auto c = sqlpp::value(a);
    auto d = sqlpp::value(b);
    auto e = c + d;

    expect_compatibility_fails<CTX, decltype(a), "Postgresql: No support for unsigned integral">();
    expect_compatibility_fails<CTX, decltype(b), "Postgresql: No support for unsigned integral">();
    expect_compatibility_fails<CTX, decltype(c), "Postgresql: No support for unsigned integral">();
    expect_compatibility_fails<CTX, decltype(d), "Postgresql: No support for unsigned integral">();
    expect_compatibility_fails<CTX, decltype(e), "Postgresql: No support for unsigned integral">();
  }
}
