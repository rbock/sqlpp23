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

#include <chrono>

#include <sqlpp23/sqlite3/sqlite3.h>
#include <sqlpp23/sqlpp23.h>

#include <sqlpp23/tests/core/assert_throw.h>
#include <sqlpp23/tests/core/result_helpers.h>
#include <sqlpp23/tests/sqlite3/make_test_connection.h>
#include <sqlpp23/tests/sqlite3/tables.h>

namespace sql = sqlpp::sqlite3;

namespace {
SQLPP_CREATE_NAME_TAG(something);
}

int main() {
  sql::connection db = sql::make_test_connection();
  const auto now = std::chrono::system_clock::now();
  std::chrono::sys_days today = std::chrono::floor<std::chrono::days>(now);

  // cast to bool
  db(select(cast(true, as(sqlpp::integral{})).as(something)));
  db(select(cast(true, as(sqlpp::floating_point{})).as(something)));

  // cast to integer
  db(select(cast(17.5, as(sqlpp::integral{})).as(something)));
  db(select(cast("17", as(sqlpp::integral{})).as(something)));

  // cast to unsigned integer
  db(select(cast(17.5, as(sqlpp::unsigned_integral{})).as(something)));
  db(select(cast("y17", as(sqlpp::unsigned_integral{})).as(something)));

  // cast to double precision
  db(select(cast(17.5, as(sqlpp::floating_point{})).as(something)));
  db(select(cast("17", as(sqlpp::floating_point{})).as(something)));

  // cast to text
  db(select(cast(17.5, as(sqlpp::text{})).as(something)));
  db(select(cast(now, as(sqlpp::text{})).as(something)));
  db(select(cast(today, as(sqlpp::text{})).as(something)));
  db(select(
      cast(std::vector<uint8_t>{'1', '2', '3', '4', '5'}, as(sqlpp::text{}))
          .as(something)));

  // cast to blob
  db(select(cast("17", as(sqlpp::blob{})).as(something)));

  // cast to date
  // cast to timestamp
  // cast to time
  // No such thing in sqlite3
}
