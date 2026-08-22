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

#include <sqlpp26/tests/sqlite3/all.h>

namespace sql = sqlpp::sqlite3;

const auto foo = test::tab_foo{};

int Integral(int, char*[]) {
  auto db = sql::make_test_connection();
  test::createtab_foo(db);

  // The connector supports uint64_t values and will always retrieve the correct
  // value from the database. Sqlite3 stores the values as int64_t internally
  // though, so big uint64_t values will be converted and the library has to
  // intepret the int64_t values correctly as uint64_t. Therefore, we test
  // uint64_t values in an out of the range of int64_t and test if they are
  // retrieved correctly from the database in both cases.
  uint64_t uint64_t_value_supported = std::numeric_limits<int64_t>::max();
  int64_t int64_t_value_max = std::numeric_limits<int64_t>::max();

  uint64_t uint64_t_value_unsupported = std::numeric_limits<uint64_t>::max();
  int64_t int64_t_value_min = std::numeric_limits<int64_t>::min();

  std::size_t size_t_value_max = std::numeric_limits<std::size_t>::max();
  std::size_t size_t_value_min = std::numeric_limits<std::size_t>::min();

  uint32_t uint32_t_value = std::numeric_limits<uint32_t>::max();
  int32_t int32_t_value = std::numeric_limits<int32_t>::max();

  db(insert_into(foo).set(
      foo.int_n = int64_t_value_max,
      foo.u_int_n = uint64_t_value_supported));

  auto prepared_insert = db.prepare(insert_into(foo).set(
      foo.int_n = parameter(foo.int_n),
      foo.u_int_n = parameter(foo.u_int_n)));
  prepared_insert.parameters.int_n = int64_t_value_min;
  prepared_insert.parameters.u_int_n = uint64_t_value_unsupported;
  db(prepared_insert);

  db(insert_into(foo).set(foo.int_n = size_t_value_min,
                                foo.u_int_n = size_t_value_max));
  db(insert_into(foo).set(foo.int_n = int32_t_value,
                                foo.u_int_n = uint32_t_value));

  auto q =
      select(foo.int_n, foo.u_int_n).from(foo);

  auto rows = db(q);

  require_equal(__LINE__, rows.front().int_n.value(), int64_t_value_max);
  require_equal(__LINE__, rows.front().u_int_n.value(),
                uint64_t_value_supported);
  rows.pop_front();

  require_equal(__LINE__, rows.front().int_n.value(), int64_t_value_min);
  require_equal(__LINE__, rows.front().u_int_n.value(),
                uint64_t_value_unsupported);
  rows.pop_front();

  require_equal(__LINE__, rows.front().int_n.value(), int64_t{});
  // the uint64_t_value_max gets truncated to int64_t_value_max
  require_equal(__LINE__, rows.front().u_int_n.value(),
                static_cast<uint64_t>(int64_t_value_max));
  rows.pop_front();

  require_equal(__LINE__, rows.front().int_n.value(), int32_t_value);
  require_equal(__LINE__, rows.front().u_int_n.value(), uint32_t_value);
  rows.pop_front();

  return 0;
}
