/*
 * Copyright (c) 2013 - 2016, Roland Bock
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

#include <cassert>
#include <iostream>

#include <sqlpp23/sqlite3/database/connection.h>
#include <sqlpp23/tests/sqlite3/make_test_connection.h>
#include <sqlpp23/sqlpp23.h>

#ifdef SQLPP_USE_SQLCIPHER
#include <sqlcipher/sqlite3.h>
#else
#include <sqlite3.h>
#endif

namespace sql = sqlpp::sqlite3;

SQLPP_CREATE_NAME_TAG(pragma);

int Transaction(int, char*[]) {
  auto db = sql::make_test_connection();

  std::cerr << "--------------------------------------" << std::endl;

  assert(db.is_transaction_active() == false);

  auto current_level = db.get_default_isolation_level();
  std::cout << "Expecting default isolation level = 1, is "
            << static_cast<int>(current_level) << std::endl;
  assert(current_level == sqlpp::isolation_level::serializable);

  int64_t pragmaValue =
      db(sqlpp::statement_t<>{}
         << sqlpp::verbatim("PRAGMA read_uncommitted")
         << with_result_type_of(select(sqlpp::value(1).as(pragma))))
          .front()
          .pragma;
  assert(pragmaValue == 0);

  std::cerr << "Expecting read_uncommitted = 0, is: " << pragmaValue
            << std::endl;
  db.set_default_isolation_level(sqlpp::isolation_level::read_uncommitted);

  auto tx = start_transaction(db);
  assert(db.is_transaction_active());

  pragmaValue = db(sqlpp::statement_t<>{}
                   << sqlpp::verbatim("PRAGMA read_uncommitted")
                   << with_result_type_of(select(sqlpp::value(1).as(pragma))))
                    .front()
                    .pragma;
  std::cerr << "Now expecting read_uncommitted = 1, is: " << pragmaValue
            << std::endl;
  assert(pragmaValue == 1);

  current_level = db.get_default_isolation_level();
  std::cout << "Now expecting default isolation level = 4, is "
            << static_cast<int>(current_level) << std::endl;
  assert(current_level == sqlpp::isolation_level::read_uncommitted);

  tx.commit();
  assert(db.is_transaction_active() == false);
  std::cerr << "--------------------------------------" << std::endl;

  return 0;
}
