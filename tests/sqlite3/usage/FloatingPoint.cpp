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

#include <cmath>

#include <sqlpp23/tests/sqlite3/all.h>

namespace sql = sqlpp::sqlite3;

const auto tab = test::TabFoo{};

static auto require(int line, bool condition) -> void {
  if (!condition) {
    std::cerr << line << " condition violated";
    throw std::runtime_error("Unexpected result");
  }
}

int FloatingPoint(int, char*[]) {
  auto db = sql::make_test_connection();
  test::createTabFoo(db);

  db("INSERT into tab_foo (id, double_n) values(NULL, 1.0)");
  db("INSERT into tab_foo (id, double_n) values(NULL, 'Inf')");
  db("INSERT into tab_foo (id, double_n) values(NULL, 'Nan')");
  db("INSERT into tab_foo (id, double_n) values(NULL, 'SomeString')");
  db(insert_into(tab).set(tab.doubleN = std::numeric_limits<double>::quiet_NaN()));
  db(insert_into(tab).set(tab.doubleN = std::numeric_limits<double>::infinity()));
  db(insert_into(tab).set(tab.doubleN = -std::numeric_limits<double>::infinity()));

  auto prepared_insert =
      db.prepare(insert_into(tab).set(tab.doubleN = parameter(tab.doubleN)));
  prepared_insert.parameters.doubleN = std::numeric_limits<double>::quiet_NaN();
  db(prepared_insert);
  prepared_insert.parameters.doubleN = std::numeric_limits<double>::infinity();
  db(prepared_insert);
  prepared_insert.parameters.doubleN = -std::numeric_limits<double>::infinity();
  db(prepared_insert);

  auto q = select(tab.doubleN).from(tab);
  auto rows = db(q);

  // raw string inserts
  require_equal(__LINE__, rows.front().doubleN, 1.0);
  rows.pop_front();
  require(__LINE__, std::isinf(rows.front().doubleN.value()));
  rows.pop_front();
  require(__LINE__, std::isnan(rows.front().doubleN.value()));
  rows.pop_front();
  require_equal(__LINE__, rows.front().doubleN, 0.0);
  rows.pop_front();

  // dsl inserts
  require(__LINE__, std::isnan(rows.front().doubleN.value()));
  rows.pop_front();
  require(__LINE__, std::isinf(rows.front().doubleN.value()));
  require(__LINE__,
          rows.front().doubleN.value() > std::numeric_limits<double>::max());
  rows.pop_front();
  require(__LINE__, std::isinf(rows.front().doubleN.value()));
  require(__LINE__,
          rows.front().doubleN.value() < std::numeric_limits<double>::lowest());

  // prepared dsl inserts
  rows.pop_front();
  require(__LINE__, std::isnan(rows.front().doubleN.value()));
  rows.pop_front();
  require(__LINE__, std::isinf(rows.front().doubleN.value()));
  require(__LINE__,
          rows.front().doubleN.value() > std::numeric_limits<double>::max());
  rows.pop_front();
  require(__LINE__, std::isinf(rows.front().doubleN.value()));
  require(__LINE__,
          rows.front().doubleN.value() < std::numeric_limits<double>::lowest());

  return 0;
}
