
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

#include <cassert>

#include <sqlpp23/postgresql/postgresql.h>
#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/tests/postgresql/make_test_connection.h>
#include <sqlpp23/tests/postgresql/tables.h>

SQLPP_CREATE_NAME_TAG(cheese);

namespace sql = sqlpp::postgresql;
int main(int, char*[]) {
  try {
    auto db = sql::make_test_connection();
    test::createTabFoo(db);

    const auto foo = test::TabFoo{};

    // select value
    for (const auto& row : db(select(sqlpp::value(23).as(cheese)))) {
      std::ignore = row.cheese;
    }

    // select single column
    for (const auto& row : db(select(foo.id).from(foo))) {
      std::ignore = row.id;
    }

    // select two columns
    for (const auto& row : db(select(foo.id, foo.textNnD).from(foo))) {
      std::ignore = row.id;
      std::ignore = row.textNnD;
    }

    // select all columns
    for (const auto& row : db(select(all_of(foo)).from(foo))) {
      std::ignore = row.id;
      std::ignore = row.textNnD;
      std::ignore = row.intN;
    }

  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
