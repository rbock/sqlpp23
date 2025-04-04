/*
 * Copyright (c) 2017, Volker Aßmann
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

#include <iostream>
#include <memory>
#include <stdexcept>

#include <sqlpp23/core/database/transaction.h>
#include <sqlpp23/core/name/create_name_tag.h>
#include <sqlpp23/postgresql/database/connection.h>
#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/tests/core/result_helpers.h>

#include <sqlpp23/tests/postgresql/make_test_connection.h>

namespace sql = sqlpp::postgresql;

SQLPP_CREATE_NAME_TAG(level);

int Transaction(int, char*[]) {
  sql::connection db = sql::make_test_connection();

  try {
    {
      require_equal(__LINE__, db.is_transaction_active(), false);
      auto current_level = std::string(
          db(sqlpp::statement_t{}
             << sqlpp::verbatim("show transaction_isolation;")
             << with_result_type_of(select(sqlpp::value("").as(level))))
              .front()
              .level);
      require_equal(__LINE__, current_level, "read committed");
      std::cerr << "isolation level outside transaction: " << current_level
                << "\n";

      auto tx = start_transaction(db, sqlpp::isolation_level::serializable);
      require_equal(__LINE__, db.is_transaction_active(), true);
      current_level =
          db(sqlpp::statement_t{}
             << sqlpp::verbatim("show transaction_isolation;")
             << with_result_type_of(select(sqlpp::value("").as(level))))
              .front()
              .level;
      require_equal(__LINE__, current_level, "serializable");
      std::cerr << "isolation level in transaction(serializable) : "
                << current_level << "\n";

      tx.commit();
      require_equal(__LINE__, db.is_transaction_active(), false);
    }

    require_equal(__LINE__, db.get_default_isolation_level(),
                  sqlpp::isolation_level::read_committed);
    db.set_default_isolation_level(sqlpp::isolation_level::serializable);
    require_equal(__LINE__, db.get_default_isolation_level(),
                  sqlpp::isolation_level::serializable);
  } catch (const sqlpp::exception& ex) {
    std::cerr << "Got exception: " << ex.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Got unknown exception" << std::endl;
    return 1;
  }
  return 0;
}
