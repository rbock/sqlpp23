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

#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/sqlite3/database/connection.h>
#include <sqlpp23/tests/sqlite3/make_test_connection.h>
#include "TabSample.h"

#ifdef SQLPP_USE_SQLCIPHER
#include <sqlcipher/sqlite3.h>
#else
#include <sqlite3.h>
#endif
#include <iostream>

#include <sqlpp23/sqlite3/dynamic_libsqlite3.h>

SQLPP_CREATE_NAME_TAG(left);

namespace sql = sqlpp::sqlite3;
int main() {
  auto db = sql::make_test_connection();
  sql::connection_config config  db.execute(
      "CREATE TABLE tab_sample (\
        alpha bigint(20) DEFAULT NULL,\
            beta varchar(255) DEFAULT NULL,\
            gamma bool\
            )");

  const auto tab = TabSample{};

  auto i = insert_into(tab).columns(tab.beta, tab.gamma);
  i.add_values(tab.beta = "rhabarbertorte", tab.gamma = false);
  // i.add_values(tab.beta = "cheesecake", tab.gamma = false)
  // i.add_values(tab.beta = "kaesekuchen", tab.gamma = true)
  auto last_insert_rowid = db(i);

  std::cerr << "last insert rowid: " << last_insert_rowid << std::endl;

  // Just to demonstrate that you can call basically any function
  std::cerr << "last insert rowid: "
            << db(select(sqlpp::verbatim<sqlpp::integer>("last_insert_rowid()")
                             .as(tab.alpha)))
                   .front()
                   .alpha
            << std::endl;

  // select a static (alpha) and a dynamic column (beta)
  auto s = dynamic_select(db).dynamic_columns(tab.alpha.as(left)).from(tab);
  s.selected_columns.add(tab.beta);
  s.selected_columns.add(tab.gamma);
  for (const auto& row : db(s)) {
    std::cerr << "row.alpha: " << row.left << ", row.beta: " << row.at("beta")
              << ", row.gamma: " << row.at("gamma") << std::endl;
  };
  return 0;
}
