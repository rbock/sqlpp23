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

#include <sqlpp23/tests/sqlite3/all.h>

int Returning(int, char*[]) {
  namespace sql = sqlpp::sqlite3;

  sql::connection db = sql::make_test_connection();

  test::TabFoo foo = {};

  try {
    test::createTabFoo(db);

    std::cout << db(sqlpp::sqlite3::insert_into(foo)
                        .set(foo.textNnD = "dsa")
                        .returning(foo.doubleN))
                     .front()
                     .doubleN
              << std::endl;

    std::cout << db(sqlpp::sqlite3::insert_into(foo)
                        .set(foo.textNnD = "asd")
                        .returning(std::make_tuple(foo.doubleN)))
                     .front()
                     .doubleN
              << std::endl;

    auto updated = db(sqlpp::sqlite3::update(foo)
                          .set(foo.intN = 0)
                          .returning(foo.textNnD, foo.intN));
    for (const auto& row : updated)
      std::cout << "Gamma: " << row.textNnD << " Beta: " << row.intN
                << std::endl;

    auto dynamic_updated =
        db(sqlpp::sqlite3::update(foo)
               .set(foo.intN = 0, foo.doubleN = std::nullopt)
               .returning(foo.textNnD, dynamic(true, foo.intN)));
    for (const auto& row : updated)
      std::cout << "Gamma: " << row.textNnD << " Beta: " << row.intN
                << std::endl;

    auto removed = db(sqlpp::sqlite3::delete_from(foo)
                          .where(foo.intN == 0)
                          .returning(foo.textNnD, foo.intN));
    for (const auto& row : removed)
      std::cout << "Gamma: " << row.textNnD << " Beta: " << row.intN
                << std::endl;

    auto multi_insert =
        sqlpp::sqlite3::insert_into(foo).columns(foo.intN).returning(
            foo.id, foo.intN);
    multi_insert.add_values(foo.intN = 1);
    multi_insert.add_values(foo.intN = 2);
    auto inserted = db(multi_insert);

    for (const auto& row : inserted)
      std::cout << row.intN << std::endl;

  }

  catch (const sqlpp::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
