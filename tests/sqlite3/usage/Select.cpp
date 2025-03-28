/*
 * Copyright (c) 2013 - 2016, Roland Bock
 * Copyright (c) 2017, Juan Dent
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
#include <vector>

#include <sqlpp23/sqlite3/database/connection.h>
#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/tests/sqlite3/make_test_connection.h>
#include "Tables.h"

namespace sql = sqlpp::sqlite3;
const auto tab = test::TabSample{};

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::optional<T>& t) {
  if (not t)
    return os << "NULL";
  return os << t.value();
}

void testSelectAll(sql::connection& db, size_t expectedRowCount) {
  std::cerr << "--------------------------------------" << std::endl;
  size_t i = 0;
  for (const auto& row : db(sqlpp::select(all_of(tab)).from(tab))) {
    ++i;
    std::cerr << ">>> row.id: " << row.id << ", row.alpha: " << row.alpha
              << ", row.beta: " << row.beta << ", row.gamma: " << row.gamma
              << std::endl;
    assert(row.id == static_cast<int64_t>(i));
  };
  assert(i == expectedRowCount);

  auto preparedSelectAll = db.prepare(sqlpp::select(all_of(tab)).from(tab));
  i = 0;
  for (const auto& row : db(preparedSelectAll)) {
    ++i;
    std::cerr << ">>> row.id: " << row.id << ", row.alpha: " << row.alpha
              << ", row.beta: " << row.beta << ", row.gamma: " << row.gamma
              << std::endl;
    assert(row.id == static_cast<int64_t>(i));
  };
  assert(i == expectedRowCount);
  std::cerr << "--------------------------------------" << std::endl;
}

namespace string_util {
std::string ltrim(std::string str, const std::string& chars = "\t\n\v\f\r ") {
  str.erase(0, str.find_first_not_of(chars));
  return str;
}

std::string rtrim(std::string str, const std::string& chars = "\t\n\v\f\r ") {
  str.erase(str.find_last_not_of(chars) + 1);
  return str;
}

std::string trim(std::string str, const std::string& chars = "\t\n\v\f\r ") {
  return ltrim(rtrim(str, chars), chars);
}
}  // namespace string_util

namespace {
SQLPP_CREATE_NAME_TAG(something);
}

int Select(int, char*[]) {
  auto db = sql::make_test_connection();
  test::createTabSample(db);

  testSelectAll(db, 0);
  db(insert_into(tab).default_values());
  testSelectAll(db, 1);
  db(insert_into(tab).set(tab.gamma = true, tab.beta = " cheesecake "));
  testSelectAll(db, 2);
  db(insert_into(tab).set(tab.gamma = true, tab.beta = " cheesecake "));
  testSelectAll(db, 3);

  // test functions and operators
  db(select(all_of(tab)).from(tab).where(tab.alpha.is_null()));
  db(select(all_of(tab)).from(tab).where(tab.alpha.is_not_null()));
  db(select(all_of(tab)).from(tab).where(tab.alpha.in(1, 2, 3)));
  db(select(all_of(tab))
         .from(tab)
         .where(tab.alpha.in(std::vector<int>{1, 2, 3, 4})));
  db(select(all_of(tab)).from(tab).where(tab.alpha.not_in(1, 2, 3)));
  db(select(all_of(tab))
         .from(tab)
         .where(tab.alpha.not_in(std::vector<int>{1, 2, 3, 4})));
  db(select(count(tab.alpha).as(something)).from(tab));
  db(select(avg(tab.alpha).as(something)).from(tab));
  db(select(max(tab.alpha).as(something)).from(tab));
  db(select(min(tab.alpha).as(something)).from(tab));
  db(select(
         exists(select(tab.alpha).from(tab).where(tab.alpha > 7)).as(something))
         .from(tab));
  db(select(trim(tab.beta).as(something)).from(tab));

  // db(select(not_exists(select(tab.alpha).from(tab).where(tab.alpha >
  // 7))).from(tab)); db(select(all_of(tab)).from(tab).where(tab.alpha ==
  // any(select(tab.alpha).from(tab).where(tab.alpha < 3))));

  db(select(all_of(tab)).from(tab).where((tab.alpha + tab.alpha) > 3));
  db(select(all_of(tab)).from(tab).where((tab.beta + tab.beta) == ""));
  db(select(all_of(tab))
         .from(tab)
         .where((tab.beta + tab.beta).like(R"(%'\"%)")));

  // update
  db(update(tab).set(tab.gamma = false).where(tab.alpha.in(1)));
  db(update(tab)
         .set(tab.gamma = false)
         .where(tab.alpha.in(std::vector<int>{1, 2, 3, 4})));

  // delete
  db(delete_from(tab).where(tab.alpha == tab.alpha + 3));

  auto result = db(select(all_of(tab)).from(tab));
  std::cerr
      << "Accessing a field directly from the result (using the current row): "
      << result.begin()->alpha << std::endl;
  std::cerr << "Can do that again, no problem: " << result.begin()->alpha
            << std::endl;

  std::cerr << "--------------------------------------" << std::endl;
  auto tx = start_transaction(db);
  for (const auto& row :
       db(select(all_of(tab),
                 value(select(max(tab.alpha).as(something)).from(tab))
                     .as(something))
              .from(tab))) {
    const auto x = row.alpha;
    const auto a = row.something;
    std::cout << ">>>" << x << ", " << a << std::endl;
  }
  for (const auto& row :
       db(select(tab.alpha, tab.beta, tab.gamma, trim(tab.beta).as(something))
              .from(tab))) {
    std::cerr << ">>> row.alpha: " << row.alpha << ", row.beta: " << row.beta
              << ", row.gamma: " << row.gamma << ", row.something: '"
              << row.something << "'" << std::endl;
    // check something
    assert((not row.beta and not row.something) ||
           string_util::trim(std::string(row.beta.value())) ==
               row.something.value());
    // end
  };

  for (const auto& row :
       db(select(all_of(tab),
                 value(select(trim(tab.beta).as(something)).from(tab))
                     .as(something))
              .from(tab))) {
    const std::optional<int64_t> x = row.alpha;
    const std::optional<std::string_view> a = row.something;
    std::cout << ">>>" << x << ", " << a << std::endl;
  }

  tx.commit();
  std::cerr << "--------------------------------------" << std::endl;

  return 0;
}
