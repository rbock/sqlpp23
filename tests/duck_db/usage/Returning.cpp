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

int Returning(int, char*[]) {
  auto db = sql::make_test_connection({});
  test::createTabFoo(db);

  const auto foo = test::TabFoo{};

  // INSERT ... RETURNING
  for (const auto& row : db(sql::insert_into(foo)
                                .set(foo.id = 1, foo.textNnD = "inserted")
                                .returning(foo.id, foo.textNnD))) {
    require_equal(row.id, 1);
    require_equal(std::string{row.textNnD}, std::string{"inserted"});
  }

  // UPDATE ... RETURNING
  for (const auto& row : db(sql::update(foo)
                                .set(foo.textNnD = "updated")
                                .where(foo.id == 1)
                                .returning(foo.id, foo.textNnD))) {
    require_equal(row.id, 1);
    require_equal(std::string{row.textNnD}, std::string{"updated"});
  }

  // DELETE ... RETURNING
  for (const auto& row :
       db(sql::delete_from(foo).where(foo.id == 1).returning(foo.id))) {
    require_equal(row.id, 1);
  }

  size_t rows = 0;
  for (const auto& row : db(select(foo.id).from(foo))) {
    (void)row;
    ++rows;
  }
  require_equal(rows, 0u);

  return 0;
}
