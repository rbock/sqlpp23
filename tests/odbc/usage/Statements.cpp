/*
 * Copyright (c) 2026, Leander Schulten
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

#include <cstdlib>

#include <sqlpp23/tests/odbc/all.h>

// Exercises all execution paths of the connector: direct and prepared
// select/insert/update/delete/execute as well as transactions.
//
// Without a configured data source this only verifies that everything
// compiles: set SQLPP_ODBC_CONNECTION_STRING to run it against a real
// database (the DDL in tables.h may need adjustment for your database).
namespace {
void run_all_statement_types(sqlpp::odbc::connection& db) {
  const auto foo = test::TabFoo{};

  test::createTabFoo(db);

  // direct execution
  db(insert_into(foo).set(foo.textNnD = "direct", foo.intN = 17,
                          foo.doubleN = 3.5, foo.boolN = true));
  for (const auto& row :
       db(select(foo.id, foo.textNnD, foo.intN, foo.doubleN, foo.boolN)
              .from(foo)
              .where(foo.intN.is_not_null()))) {
    std::println("id: {}, text: {}, int: {}", row.id, row.textNnD,
                 row.intN.value_or(0));
  }
  db(update(foo).set(foo.intN = 18).where(foo.textNnD == "direct"));
  db(delete_from(foo).where(foo.intN == 18));

  // prepared execution
  auto prepared_insert = db.prepare(insert_into(foo).set(
      foo.textNnD = parameter(foo.textNnD), foo.intN = parameter(foo.intN)));
  prepared_insert.parameters.textNnD = "prepared";
  prepared_insert.parameters.intN = 42;
  db(prepared_insert);
  prepared_insert.parameters.intN = std::nullopt;  // NULL
  db(prepared_insert);

  auto prepared_select = db.prepare(
      select(foo.id, foo.textNnD).from(foo).where(foo.id > parameter(foo.id)));
  prepared_select.parameters.id = 0;
  for (const auto& row : db(prepared_select)) {
    std::println("id: {}, text: {}", row.id, row.textNnD);
  }

  auto prepared_update = db.prepare(
      update(foo).set(foo.intN = 43).where(foo.textNnD == parameter(foo.textNnD)));
  prepared_update.parameters.textNnD = "prepared";
  db(prepared_update);

  auto prepared_delete = db.prepare(
      delete_from(foo).where(foo.textNnD == parameter(foo.textNnD)));
  prepared_delete.parameters.textNnD = "prepared";
  db(prepared_delete);

  // transactions
  {
    auto tx = start_transaction(db);
    db(insert_into(foo).set(foo.textNnD = "in transaction"));
    tx.commit();
  }
  {
    auto tx = start_transaction(db);
    db(insert_into(foo).set(foo.textNnD = "rolled back"));
    tx.rollback();
  }
}
}  // namespace

int main() {
  if (std::getenv("SQLPP_ODBC_CONNECTION_STRING") == nullptr) {
    std::println(
        "ODBC: skipping statement execution, SQLPP_ODBC_CONNECTION_STRING is "
        "not set");
    return 0;
  }

  try {
    auto db = sqlpp::odbc::make_test_connection();
    run_all_statement_types(db);
  } catch (const std::exception& e) {
    std::println(stderr, "Exception: {}", e.what());
    return -1;
  }

  return 0;
}
