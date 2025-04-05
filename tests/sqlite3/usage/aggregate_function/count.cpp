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

#include <sqlpp23/sqlite3/sqlite3.h>
#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/tests/sqlite3/make_test_connection.h>
#include "../Tables.h"

SQLPP_CREATE_NAME_TAG(count_1);
SQLPP_CREATE_NAME_TAG(count_star);

auto require_close(int line, double l, double r) -> void
{
  if (std::abs(l - r) > 0.001) {
    std::cerr << line << ": abs(";
    std::cerr << sqlpp::to_sql_string(std::cerr, l);
    std::cerr << " - ";
    std::cerr << sqlpp::to_sql_string(std::cerr, r);
    std::cerr << ") > 0.001\n";
    throw std::runtime_error("Unexpected result");
  }
}

namespace sql = sqlpp::sqlite3;
int main(int, char*[]) {
  try {
    const auto tab = test::TabSample{};
    auto db = sql::make_test_connection();

    test::createTabSample(db);

    // clear the table
    db(truncate(tab));

    // insert
    db(insert_into(tab).set(tab.alpha = 7));
    db(insert_into(tab).set(tab.alpha = 7));
    db(insert_into(tab).set(tab.alpha = 9));

    // select count
    for (const auto& row : db(select(
            count(tab.alpha).as(sqlpp::alias::count_),
            sqlpp::count(1).as(count_1),
            count(sqlpp::star).as(count_star)
            ).from(tab))) {
      assert(row.count_ == 3);
      assert(row.count_1 == 3);
      assert(row.count_star == 3);
    }

    // select distinct count
    for (const auto& row : db(select(
            count(sqlpp::distinct, tab.alpha).as(sqlpp::alias::count_),
            count(sqlpp::distinct, 1).as(count_1)
            ).from(tab))) {
      assert(row.count_ == 2);
      assert(row.count_1 == 1);
    }
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
