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
#include "sqlpp23/core/type_traits/data_type.h"

#include <sqlpp23/postgresql/postgresql.h>
#include <sqlpp23/sqlpp23.h>

#include <sqlpp23/tests/postgresql/tables.h>
#include <sqlpp23/tests/postgresql/make_test_connection.h>
#include <sqlpp23/tests/core/result_helpers.h>
#include <sqlpp23/tests/core/assert_throw.h>

namespace sql = sqlpp::postgresql;

namespace {
SQLPP_CREATE_NAME_TAG(something);
}

int main() {
  sql::connection db = sql::make_test_connection();
  const auto now = std::chrono::system_clock::now();
  std::chrono::sys_days today = std::chrono::floor<std::chrono::days>(now);

  // cast to bool
  db(select(cast("t", as(sqlpp::boolean{})).as(something)));
  db(select(cast(17, as(sqlpp::boolean{})).as(something)));
  assert_throw(db(select(cast(17.5, as(sqlpp::boolean{})).as(something))), sql::result_exception);
  db(select(cast(std::nullopt, as(sqlpp::boolean{})).as(something)));
  assert_throw(db(select(cast("nonsense", as(sqlpp::boolean{})).as(something))), sql::result_exception);

  // cast to integer
  db(select(cast(17.5, as(sqlpp::integral{})).as(something)));
  db(select(cast("17", as(sqlpp::integral{})).as(something)));
  assert_throw(db(select(cast("17.5", as(sqlpp::integral{})).as(something))), sql::result_exception);

  // cast to double precision
  db(select(cast(17.5, as(sqlpp::floating_point{})).as(something)));
  db(select(cast("17", as(sqlpp::floating_point{})).as(something)));
  db(select(cast("17.5", as(sqlpp::floating_point{})).as(something)));

  // cast to text
  db(select(cast(17.5, as(sqlpp::text{})).as(something)));
  db(select(cast("17", as(sqlpp::text{})).as(something)));
  db(select(cast(now, as(sqlpp::text{})).as(something)));
  db(select(cast(today, as(sqlpp::text{})).as(something)));
  db(select(cast(std::vector<uint8_t>{1,2,3,4,5}, as(sqlpp::text{})).as(something)));

  // cast to blob
  db(select(cast("17", as(sqlpp::blob{})).as(something)));

  // cast to date
  db(select(cast(now, as(sqlpp::date{})).as(something)));
  db(select(cast("2025-05-31 08:29:44.196353786+00", as(sqlpp::date{})).as(something)));
  db(select(cast("2025-05-31", as(sqlpp::date{})).as(something)));

  // cast to timestamp
  db(select(cast(today, as(sqlpp::timestamp{})).as(something)));
  db(select(cast("2025-05-31 08:29:44.196353786+00", as(sqlpp::timestamp{})).as(something)));
  db(select(cast("2025-05-31", as(sqlpp::timestamp{})).as(something)));

  // cast to time
  db(select(cast("08:29:44.196353786", as(sqlpp::time{})).as(something)));
}
