/*
 * Copyright (c) 2013 - 2015, Roland Bock
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

#include <cassert>
#include <iostream>

#include <sqlpp23/postgresql/postgresql.h>
#include <sqlpp23/sqlpp23.h>

#include <sqlpp23/tests/postgresql/tables.h>
#include <sqlpp23/tests/postgresql/make_test_connection.h>

namespace {
const auto now = std::chrono::floor<::std::chrono::microseconds>(
    std::chrono::system_clock::now());
const auto today = std::chrono::floor<std::chrono::days>(now);
const auto yesterday = today - std::chrono::days{1};
const auto current = now - today;

template <typename L, typename R>
auto require_equal(int line, const L& l, const R& r) -> void {
  if (l != r) {
    std::cerr << line << ": ";
    std::cerr << sqlpp::to_sql_string(std::cerr, l);
    std::cerr << " != ";
    std::cerr << sqlpp::to_sql_string(std::cerr, r);
    throw std::runtime_error("Unexpected result");
  }
}
}  // namespace

int DateTime(int, char*[]) {
  namespace sql = sqlpp::postgresql;

  sql::connection db = sql::make_test_connection();

  test::createTabDateTime(db);

  test::TabDateTime tab = {};
  try {
    db(insert_into(tab).default_values());
    for (const auto& row : db(select(all_of(tab)).from(tab))) {
      require_equal(__LINE__, row.dateN.has_value(), false);
      require_equal(__LINE__, row.timeNTz.has_value(), false);
      require_equal(__LINE__, row.timestampNTz.has_value(), false);
    }

    db(update(tab).set(tab.dateN = today, tab.timeNTz = current,
                       tab.timestampNTz = now));

    for (const auto& row : db(select(all_of(tab)).from(tab))) {
      require_equal(__LINE__, row.dateN.value(), today);
      require_equal(__LINE__, row.timeNTz.value(), current);
      require_equal(__LINE__, row.timestampNTz.value(), now);
    }

    db(update(tab).set(tab.dateN = yesterday, tab.timestampNTz = now));

    for (const auto& row : db(select(all_of(tab)).from(tab))) {
      require_equal(__LINE__, row.dateN.value(), yesterday);
      require_equal(__LINE__, row.timeNTz.value(), current);
      require_equal(__LINE__, row.timestampNTz.value(), now);
    }

    auto prepared_update = db.prepare(
        update(tab).set(tab.dateN = parameter(tab.dateN),
                        tab.timeNTz = parameter(tab.timeNTz),
                        tab.timestampNTz = parameter(tab.timestampNTz)));
    prepared_update.parameters.dateN = today;
    prepared_update.parameters.timeNTz = current;
    prepared_update.parameters.timestampNTz = now;
    std::cout << "---- running prepared update ----" << std::endl;
    db(prepared_update);
    std::cout << "---- finished prepared update ----" << std::endl;
    for (const auto& row : db(select(all_of(tab)).from(tab))) {
      require_equal(__LINE__, row.dateN.value(), today);
      require_equal(__LINE__, row.timeNTz.value(), current);
      require_equal(__LINE__, row.timestampNTz.value(), now);
    }
  } catch (const sqlpp::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
