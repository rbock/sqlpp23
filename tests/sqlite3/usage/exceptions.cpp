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

#include <print>

#include <sqlpp23/sqlite3/sqlite3.h>
#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/tests/core/assert_throw.h>
#include <sqlpp23/tests/sqlite3/make_test_connection.h>
#include <sqlpp23/tests/sqlite3/tables.h>

namespace sql = sqlpp::sqlite3;
int main() {
  try {
    {
      // broken_connection exception on bad config
      auto config = std::make_shared<sql::connection_config>();
      config->path_to_database = "non-existing-file";
      assert_throw(sql::connection{config}, sql::exception);
    }

    sql::connection db = sql::make_test_connection();
    test::createTabFoo(db);

    constexpr auto foo = test::TabFoo{};

    assert_throw(db(insert_into(foo).set(
                     foo.intN = sqlpp::verbatim<sqlpp::integral>("nonsense"))),
                 sql::exception);

    // Test fields of a result_exception
    try {
      // Broken primary key constraint
      db(insert_into(foo).set(foo.id = 7));
      db(insert_into(foo).set(foo.id = 7));
      throw std::runtime_error("Whoopsie, that broken insert worked?");
    } catch (const sql::exception& e) {
      std::println("Caught expected error.\nmessage: {}\ncode: {}",
                   e.what(), e.error_code());
      if (e.error_code() != SQLITE_CONSTRAINT_PRIMARYKEY){
        throw std::runtime_error("unexpected meta information in exception");
      }
    }
  } catch (const std::exception& e) {
    std::println("unexpected exception: {}", e.what());
    return 1;
  }
}
