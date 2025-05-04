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
#include "sqlpp23/core/consistent.h"
#ifdef SQLPP_USE_SQLCIPHER
#include <sqlcipher/sqlite3.h>
#else
#include <sqlite3.h>
#endif
#if SQLITE_VERSION_NUMBER >= 3008003
#undef SQLITE_VERSION_NUMBER
#define SQLITE_VERSION_NUMBER 3008002
#endif

#include <sqlpp23/tests/core/constraints_helpers.h>

#include <sqlpp23/sqlite3/sqlite3.h>
#include <sqlpp23/tests/core/tables.h>
#include <sqlpp23/tests/sqlite3/make_test_connection.h>

namespace {
SQLPP_CREATE_NAME_TAG(something);
}

int main() {
  auto db = sqlpp::sqlite3::make_test_connection();
  auto ctx = sqlpp::sqlite3::context_t{&db};
  using CTX = decltype(ctx);

  const auto foo = test::TabFoo{};

  {
    const auto c = cte(something).as(select(foo.id).from(foo));
    const auto w = with(c);

    static_assert(std::is_same<decltype(check_compatibility<CTX>(c)),
                               sqlpp::consistent_t>::value);
    static_assert(std::is_same<decltype(check_compatibility<CTX>(w)),
                               sqlpp::sqlite3::assert_no_with_t>::value);
  }
}
