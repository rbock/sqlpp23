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

int InsertOnConflict(int, char*[]) {
  auto db = sql::make_test_connection({});
  test::createTabFoo(db);

  const auto foo = test::TabFoo{};

  // ON CONFLICT ... DO NOTHING keeps the existing row.
  db(sql::insert_into(foo).set(foo.id = 1, foo.textNnD = "original"));
  db(sql::insert_into(foo)
         .set(foo.id = 1, foo.textNnD = "ignored")
         .on_conflict(foo.id)
         .do_nothing());

  for (const auto& row : db(select(foo.textNnD).from(foo).where(foo.id == 1))) {
    require_equal(std::string{row.textNnD}, std::string{"original"});
  }

  // ON CONFLICT ... DO UPDATE overwrites it.
  db(sql::insert_into(foo)
         .set(foo.id = 1, foo.textNnD = "updated")
         .on_conflict(foo.id)
         .do_update(foo.textNnD = "updated"));

  for (const auto& row : db(select(foo.textNnD).from(foo).where(foo.id == 1))) {
    require_equal(std::string{row.textNnD}, std::string{"updated"});
  }

  // insert_or_replace and insert_or_ignore are shorthands for the same thing.
  db(sql::insert_or_replace().into(foo).set(foo.id = 1, foo.textNnD = "replaced"));
  for (const auto& row : db(select(foo.textNnD).from(foo).where(foo.id == 1))) {
    require_equal(std::string{row.textNnD}, std::string{"replaced"});
  }

  db(sql::insert_or_ignore().into(foo).set(foo.id = 1, foo.textNnD = "ignored"));
  for (const auto& row : db(select(foo.textNnD).from(foo).where(foo.id == 1))) {
    require_equal(std::string{row.textNnD}, std::string{"replaced"});
  }

  return 0;
}
