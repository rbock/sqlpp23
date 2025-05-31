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

#warning: Need serialize tests

  // cast to bool
  db(select(cast_as("t", sqlpp::boolean{}).as(something)));
  db(select(cast_as(17, sqlpp::boolean{}).as(something)));
  assert_throw(db(select(cast_as(17.5, sqlpp::boolean{}).as(something))), sql::result_exception);
  db(select(cast_as(std::nullopt, sqlpp::boolean{}).as(something)));
  assert_throw(db(select(cast_as("nonsense", sqlpp::boolean{}).as(something))), sql::result_exception);

  // cast to integer
  db(select(cast_as(17.5, sqlpp::integral{}).as(something)));
  db(select(cast_as("17", sqlpp::integral{}).as(something)));
  assert_throw(db(select(cast_as("17.5", sqlpp::integral{}).as(something))), sql::result_exception);

  // cast to double precision
  db(select(cast_as(17.5, sqlpp::floating_point{}).as(something)));
  db(select(cast_as("17", sqlpp::floating_point{}).as(something)));
  db(select(cast_as("17.5", sqlpp::floating_point{}).as(something)));

  // cast to text
  db(select(cast_as(17.5, sqlpp::text{}).as(something)));
  db(select(cast_as("17", sqlpp::text{}).as(something)));
  db(select(cast_as(now, sqlpp::text{}).as(something)));
  db(select(cast_as(today, sqlpp::text{}).as(something)));
  db(select(cast_as(std::vector<uint8_t>{1,2,3,4,5}, sqlpp::text{}).as(something)));

  // cast to blob
  db(select(cast_as("17", sqlpp::blob{}).as(something)));

  // cast to date
  db(select(cast_as(now, sqlpp::date{}).as(something)));
  db(select(cast_as("2025-05-31 08:29:44.196353786+00", sqlpp::date{}).as(something)));
  db(select(cast_as("2025-05-31", sqlpp::date{}).as(something)));

  // cast to timestamp
  db(select(cast_as(today, sqlpp::timestamp{}).as(something)));
  db(select(cast_as("2025-05-31 08:29:44.196353786+00", sqlpp::timestamp{}).as(something)));
  db(select(cast_as("2025-05-31", sqlpp::timestamp{}).as(something)));

  // cast to time
  db(select(cast_as("08:29:44.196353786", sqlpp::time{}).as(something)));
}
