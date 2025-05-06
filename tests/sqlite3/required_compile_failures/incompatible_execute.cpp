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

#include <sqlpp23/sqlite3/database/connection.h>
#include <sqlpp23/sqlite3/sqlite3.h>
#include <sqlpp23/tests/sqlite3/make_test_connection.h>
#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/tests/core/incompatible.h>
#include <sqlpp23/tests/sqlite3/tables.h>

int main() {
  using CTX = sqlpp::sqlite3::context_t;

  // Incompatible expression
  auto incompatible_select =
      sqlpp::select(sqlpp::test::incompatible(7).as(sqlpp::alias::a));
  static_assert(
      std::is_same<decltype(check_basic_consistency(incompatible_select)),
                   sqlpp::consistent_t>::value);
  static_assert(
      std::is_same<decltype(check_prepare_consistency(incompatible_select)),
                   sqlpp::consistent_t>::value);
  static_assert(
      std::is_same<decltype(check_run_consistency(incompatible_select)),
                   sqlpp::consistent_t>::value);
  static_assert(
      std::is_same<decltype(check_compatibility<CTX>(incompatible_select)),
                   sqlpp::test::assert_no_incompatible_t>::value);

#ifdef SQLPP_CHECK_STATIC_ASSERT
  auto db = sqlpp::sqlite3::make_test_connection();
  for (const auto& row : db(incompatible_select)) {
    std::ignore = row.a;
  }
#endif
}
