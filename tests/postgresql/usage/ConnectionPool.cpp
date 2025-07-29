/*
Copyright (c) 2017 - 2018, Roland Bock
Copyright (c) 2023, Vesselin Atanasov
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#if BUILD_WITH_MODULES
import sqlpp23.core;
import sqlpp23.postgresql;
#else
#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/postgresql/postgresql.h>
#endif

#include <sqlpp23/tests/postgresql/make_test_connection.h>
#include <sqlpp23/tests/core/connection_pool_tests.h>

int ConnectionPool(int, char*[]) {
  namespace sql = sqlpp::postgresql;
  namespace test = sqlpp::test;

  try {
    test::test_connection_pool<sql::connection_pool>(
        sql::make_test_config(),
        "CREATE TABLE tab_department ("
        "id SERIAL PRIMARY KEY, "
        "name CHAR(100), "
        "division VARCHAR(255) NOT NULL DEFAULT 'engineering'"
        ")",
        PQisthreadsafe());
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
