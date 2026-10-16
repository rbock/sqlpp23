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

#include <sqlpp23/tests/duck_db/all.h>

namespace sql = sqlpp::duck_db;

int Transaction(int, char*[]) {
  auto db = sql::make_test_connection({});
  test::createTabFoo(db);
  test::createTabBar(db);

  const auto tab = test::TabFoo{};

  auto count_rows = [&db, &tab] {
    size_t rows = 0;
    for (const auto& row : db(select(tab.id).from(tab))) {
      (void)row;
      ++rows;
    }
    return rows;
  };

  // DuckDB uses snapshot isolation and has no configurable per-connection
  // isolation level.
  require_equal(db.get_default_isolation_level(),
                sqlpp::isolation_level::serializable);

  {
    auto tx = start_transaction(db);
    db(sql::insert_into(tab).set(tab.id = 1, tab.textNnD = "committed"));
    tx.commit();
    require_equal(count_rows(), 1u);
  }

  {
    auto tx = start_transaction(db);
    db(sql::insert_into(tab).set(tab.id = 2, tab.textNnD = "rolled back"));
    tx.rollback();
    require_equal(count_rows(), 1u);
  }

  {
    auto tx = start_transaction(db);
    std::ignore = tx;
    db(sql::insert_into(tab).set(tab.id = 3, tab.textNnD = "auto rollback"));
  }
  require_equal(count_rows(), 1u);

  return 0;
}
