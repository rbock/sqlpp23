/*
 * Copyright (c) 2015 - 2016, Roland Bock
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

namespace sql = sqlpp::sqlite3;

int Attach(int, char*[]) {
  // Opening a connection to an in-memory database and creating a table in it
  auto config = sql::make_test_config();
  auto db = sql::make_test_connection();
  test::createTabFoo(db);

  // Attaching another in-memory database and creating the same table in it
  auto other = db.attach(*config, "other");
  db(R"(CREATE TABLE other.tab_foo (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  text_nn_d varchar(255) NOT NULL DEFAULT '',
  int_n bigint,
  double_n double precision,
  u_int_n bigint UNSIGNED,
  bool_n bool,
  blob_n blob
))");

  auto left = test::TabFoo{};
  auto right =
      schema_qualified_table(other, test::TabFoo{})
          .as(sqlpp::alias::right);  // this is a table in the attached database

  // inserting in one tab_sample
  db(insert_into(left).default_values());

  // selecting from the other tab_sample
  assert(db(select(all_of(right)).from(right)).empty());

  return 0;
}
