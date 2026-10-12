/*
 * Copyright (c) 2026
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

// DuckDB supports RETURNING, so this must compile and execute.

#include <sqlpp23/tests/duck_db/all.h>

namespace sql = sqlpp::duck_db;

int main() {
  auto db = sql::make_test_connection({});

  test::TabFoo foo = {};

  test::createTabFoo(db);

  for (const auto& row : db(sql::insert_into(foo)
                                .set(foo.textNnD = "dsa")
                                .returning(foo.id, foo.textNnD))) {
    std::ignore = row.id;
    std::ignore = row.textNnD;
  }

  for (const auto& row : db(sql::update(foo)
                                .set(foo.intN = 0)
                                .where(foo.textNnD == "dsa")
                                .returning(foo.id, foo.intN))) {
    std::ignore = row.id;
    std::ignore = row.intN;
  }

  for (const auto& row : db(sql::delete_from(foo)
                                .where(foo.textNnD == "dsa")
                                .returning(foo.id))) {
    std::ignore = row.id;
  }

  return 0;
}
