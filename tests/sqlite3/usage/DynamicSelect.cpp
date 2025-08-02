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

#include <sqlpp23/tests/sqlite3/all.h>

namespace sql = sqlpp::sqlite3;
int DynamicSelect(int, char*[]) {
  auto db = sql::make_test_connection();
  test::createTabFoo(db);

  const auto tab = test::TabFoo{};

  auto i = insert_into(tab).columns(tab.textNnD, tab.boolN);
  i.add_values(tab.textNnD = "rhabarbertorte", tab.boolN = false);
  i.add_values(tab.textNnD = "cheesecake", tab.boolN = false);
  i.add_values(tab.textNnD = "kaesekuchen", tab.boolN = true);
  const auto last_insert_rowid = db(i).last_insert_id;

  std::cerr << "last insert rowid: " << last_insert_rowid << std::endl;

  // Just to demonstrate that you can call basically any function
  std::cerr << "last insert rowid: "
            << db(select(sqlpp::verbatim<sqlpp::integral>("last_insert_rowid()")
                             .as(tab.intN)))
                   .front()
                   .intN
            << std::endl;

  std::cerr << "last insert rowid: "
            << db(select(dynamic(true, sqlpp::verbatim<sqlpp::integral>(
                                           "last_insert_rowid()")
                                           .as(tab.intN))))
                   .front()
                   .intN
            << std::endl;

  std::cerr << "last insert rowid: "
            << db(select(dynamic(false, sqlpp::verbatim<sqlpp::integral>(
                                            "last_insert_rowid()")
                                            .as(tab.intN))))
                   .front()
                   .intN
            << std::endl;

  return 0;
}
