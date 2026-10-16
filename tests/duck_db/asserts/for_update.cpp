/*
 * Copyright (c) 2026
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
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

// DuckDB parses FOR UPDATE and FOR SHARE but rejects them with
// "SELECT locking clause is not supported", so the connector must reject them
// at compile time.

#include <sqlpp23/tests/duck_db/all.h>

int main() {
  using CTX = sqlpp::duck_db::context_t;

  const auto foo = test::TabFoo{};

  auto for_update_select = sqlpp::select(foo.id).from(foo).for_update();
  static_assert(
      std::is_same<decltype(check_basic_consistency(for_update_select)),
                   sqlpp::consistent_t>::value);
  static_assert(
      std::is_same<decltype(check_prepare_consistency(for_update_select)),
                   sqlpp::consistent_t>::value);
  static_assert(
      std::is_same<decltype(check_run_consistency(for_update_select)),
                   sqlpp::consistent_t>::value);
  static_assert(
      std::is_same<decltype(check_compatibility<CTX>(for_update_select)),
                   sqlpp::duck_db::assert_no_for_update_t>::value);

#ifdef SQLPP_CHECK_STATIC_ASSERT
  auto db = sqlpp::duck_db::make_test_connection({});
  std::ignore = db.prepare(for_update_select);
#endif
}
