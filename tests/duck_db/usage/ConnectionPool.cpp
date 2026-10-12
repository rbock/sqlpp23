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

#include <cstdio>
#include <string>

#include <sqlpp23/tests/duck_db/all.h>

namespace sql = sqlpp::duck_db;

int ConnectionPool(int, char*[]) {
  // DuckDB in-memory databases are private to a single connection, so pooled
  // connections only share data when they point at a file.
  auto path = std::string{"sqlpp23_duck_db_connection_pool_test.duckdb"};
  std::remove(path.c_str());
  std::remove((path + ".wal").c_str());

  {
    auto config = sql::make_test_file_config(path);
    sql::connection_pool pool{config, 2};

    {
      auto db = pool.get();
      test::createTabDepartment(db);
    }

    for (int i = 1; i <= 5; ++i) {
      auto db = pool.get();
      const auto tab = test::TabDepartment{};
      db(sql::insert_into(tab).set(tab.id = i, tab.division = "engineering"));
    }

    {
      auto db = pool.get();
      const auto tab = test::TabDepartment{};
      size_t rows = 0;
      for (const auto& row : db(select(tab.id).from(tab))) {
        (void)row;
        ++rows;
      }
      require_equal(rows, 5u);
    }

    // The connection check levels must all yield usable connections.
    for (const auto check : {sqlpp::connection_check::none,
                             sqlpp::connection_check::passive,
                             sqlpp::connection_check::ping}) {
      auto db = pool.get(check);
      require_equal(db.is_connected(), true);
      require_equal(db.ping_server(), true);
    }
  }

  std::remove(path.c_str());
  std::remove((path + ".wal").c_str());

  return 0;
}
