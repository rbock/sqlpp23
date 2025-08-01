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

#include <sqlpp23/tests/postgresql/all.h>

int main() {
  auto db = sqlpp::postgresql::make_test_connection();
  auto ctx = sqlpp::postgresql::context_t{&db};
  using CTX = decltype(ctx);

  // OK
  {
    auto ci = cast(std::nullopt, as(sqlpp::boolean{}));

    static_assert(
        std::is_same<decltype(check_compatibility<CTX>(ci)),
                     sqlpp::consistent_t>::value);
  }

  // Postgresql cannot cast bool to numeric
  {
    auto ci = cast(true, as(sqlpp::integral{}));
    auto cf = cast(true, as(sqlpp::floating_point{}));

    static_assert(
        std::is_same<decltype(check_compatibility<CTX>(ci)),
                     sqlpp::postgresql::assert_no_cast_bool_to_numeric>::value);
    static_assert(
        std::is_same<decltype(check_compatibility<CTX>(cf)),
                     sqlpp::postgresql::assert_no_cast_bool_to_numeric>::value);
  }

  // Postgresql cannot cast to unsigned (generally no support for unsigned).
  {
    auto cn = cast(std::nullopt, as(sqlpp::unsigned_integral{}));
    auto ci = cast(7, as(sqlpp::unsigned_integral{}));
    auto cb = cast(7.5, as(sqlpp::unsigned_integral{}));

    static_assert(
        std::is_same<decltype(check_compatibility<CTX>(cn)),
                     sqlpp::postgresql::assert_no_unsigned>::value);
    static_assert(
        std::is_same<decltype(check_compatibility<CTX>(ci)),
                     sqlpp::postgresql::assert_no_unsigned>::value);
    static_assert(
        std::is_same<decltype(check_compatibility<CTX>(cb)),
                     sqlpp::postgresql::assert_no_unsigned>::value);
  }
}
