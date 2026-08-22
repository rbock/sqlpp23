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

#include <sqlpp26/tests/postgresql/all.h>

int Returning(int, char*[]) {
  namespace sql = sqlpp::postgresql;

  sql::connection db = sql::make_test_connection();

  test::tab_foo foo = {};

  try {
    test::createtab_foo(db);

    std::cout << db(sqlpp::postgresql::insert_into(foo)
                        .set(foo.text_nn_d = "dsa")
                        .returning(foo.float_n))
                     .front()
                     .float_n
              << std::endl;

    std::cout << db(sqlpp::postgresql::insert_into(foo)
                        .set(foo.text_nn_d = "asd")
                        .returning(std::make_tuple(foo.float_n)))
                     .front()
                     .float_n
              << std::endl;

    auto updated = db(sqlpp::postgresql::update(foo)
                          .set(foo.int_n = 0)
                          .returning(foo.text_nn_d, foo.int_n));
    for (const auto& row : updated)
      std::cout << "Gamma: " << row.text_nn_d << " Beta: " << row.int_n
                << std::endl;

    auto dynamic_updated =
        db(sqlpp::postgresql::update(foo)
               .set(foo.int_n = 0, foo.float_n = std::nullopt)
               .returning(foo.text_nn_d, dynamic(true, foo.int_n)));
    for (const auto& row : updated)
      std::cout << "Gamma: " << row.text_nn_d << " Beta: " << row.int_n
                << std::endl;

    auto removed = db(sqlpp::postgresql::delete_from(foo)
                          .where(foo.int_n == 0)
                          .returning(foo.text_nn_d, foo.int_n));
    for (const auto& row : removed)
      std::cout << "Gamma: " << row.text_nn_d << " Beta: " << row.int_n
                << std::endl;

    auto multi_insert =
        sqlpp::postgresql::insert_into(foo).columns(foo.int_n).returning(
            foo.id, foo.int_n);
    multi_insert.add_values(foo.int_n = 1);
    multi_insert.add_values(foo.int_n = 2);
    auto inserted = db(multi_insert);

    for (const auto& row : inserted)
      std::cout << row.int_n << std::endl;

  }
  catch (const sqlpp::exception&) {
    return 1;
  }

  return 0;
}
