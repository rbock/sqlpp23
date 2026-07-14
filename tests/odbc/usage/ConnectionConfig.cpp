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

#include <sqlpp23/tests/odbc/all.h>

int main() {
  namespace sql = sqlpp::odbc;

  // The two ways of connecting are distinct alternatives of a variant.
  sql::connection_config config;
  assert(std::holds_alternative<sql::data_source>(config.source));

  config.source = sql::data_source{.name = "test_dsn",
                                   .username = "user",
                                   .password = "secret"};
  assert(std::holds_alternative<sql::data_source>(config.source));
  assert(std::get<sql::data_source>(config.source).name == "test_dsn");

  config.source = sql::connection_string{"Driver=Foo;Database=bar;"};
  assert(std::holds_alternative<sql::connection_string>(config.source));

  // Configurations compare by value (the debug logger is not compared).
  sql::connection_config other;
  assert(config != other);
  other.source = sql::connection_string{"Driver=Foo;Database=bar;"};
  assert(config == other);
  other.row_array_size = config.row_array_size + 1;
  assert(config != other);

  // A default-constructed connection is not connected.
  sql::connection db;
  assert(not db.is_connected());

  // Connecting to a data source that does not exist reports an ODBC error
  // with SQLSTATE and diagnostic message.
  try {
    auto bad_config = std::make_shared<sql::connection_config>();
    bad_config->source =
        sql::data_source{.name = "sqlpp23_no_such_dsn", .username = {}, .password = {}};
    db.connect_using(bad_config);
    std::cerr << "Expected connect to throw\n";
    return -1;
  } catch (const sql::exception& e) {
    // IM002: data source name not found (exact state depends on the driver
    // manager, so only check that a state was reported).
    assert(not e.sql_state().empty());
  }

  return 0;
}
