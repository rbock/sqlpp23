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

#include <sqlpp26/tests/sqlite3/all.h>

namespace sql = sqlpp::sqlite3;
const auto tab = test::tab_foo{};

void testSelectAll(sql::connection& db, size_t expectedRowCount) {
  std::cerr << "--------------------------------------" << std::endl;
  size_t i = 0;
  for (const auto& row : db(sqlpp::select(all_of(tab)).from(tab))) {
    ++i;
    std::cerr << ">>> row.id: " << row.id << ", row.int_n: " << row.int_n
              << ", row.text_nn_d: " << row.text_nn_d << ", row.bool_n: " << row.bool_n
              << std::endl;
    assert(row.id == static_cast<int64_t>(i));
  };
  assert(i == expectedRowCount);

  auto preparedSelectAll = db.prepare(sqlpp::select(all_of(tab)).from(tab));
  i = 0;
  for (const auto& row : db(preparedSelectAll)) {
    ++i;
    std::cerr << ">>> row.id: " << row.id << ", row.int_n: " << row.int_n
              << ", row.text_nn_d: " << row.text_nn_d << ", row.bool_n: " << row.bool_n
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
  test::createtab_foo(db);

  testSelectAll(db, 0);
  db(insert_into(tab).default_values());
  testSelectAll(db, 1);
  db(insert_into(tab).set(tab.bool_n = true, tab.text_nn_d = " cheesecake "));
  testSelectAll(db, 2);
  db(insert_into(tab).set(tab.bool_n = true, tab.text_nn_d = " cheesecake "));
  testSelectAll(db, 3);

  // test functions and operators
  db(select(all_of(tab)).from(tab).where(tab.int_n.is_null()));
  db(select(all_of(tab)).from(tab).where(tab.int_n.is_not_null()));
  db(select(all_of(tab)).from(tab).where(tab.int_n.in(1, 2, 3)));
  db(select(all_of(tab))
         .from(tab)
         .where(tab.int_n.in(std::vector<int>{1, 2, 3, 4})));
  db(select(all_of(tab)).from(tab).where(tab.int_n.not_in(1, 2, 3)));
  db(select(all_of(tab))
         .from(tab)
         .where(tab.int_n.not_in(std::vector<int>{1, 2, 3, 4})));
  db(select(count(tab.int_n).as(something)).from(tab));
  db(select(avg(tab.int_n).as(something)).from(tab));
  db(select(max(tab.int_n).as(something)).from(tab));
  db(select(min(tab.int_n).as(something)).from(tab));
  db(select(
         exists(select(tab.int_n).from(tab).where(tab.int_n > 7)).as(something))
         .from(tab));
  db(select(trim(tab.text_nn_d).as(something)).from(tab));
  db(select(coalesce(tab.text_nn_d, "fallback").as(something)).from(tab));

  // db(select(not_exists(select(tab.int_n).from(tab).where(tab.int_n >
  // 7))).from(tab)); db(select(all_of(tab)).from(tab).where(tab.int_n ==
  // any(select(tab.int_n).from(tab).where(tab.int_n < 3))));

  db(select(all_of(tab)).from(tab).where((tab.int_n + tab.int_n) > 3));
  db(select(all_of(tab)).from(tab).where((tab.text_nn_d + tab.text_nn_d) == ""));
  db(select(all_of(tab))
         .from(tab)
         .where((tab.text_nn_d + tab.text_nn_d).like(R"(%'\"%)")));

  // update
  db(update(tab).set(tab.bool_n = false).where(tab.int_n.in(1)));
  db(update(tab)
         .set(tab.bool_n = false)
         .where(tab.int_n.in(std::vector<int>{1, 2, 3, 4})));

  // delete
  db(delete_from(tab).where(tab.int_n == tab.int_n + 3));

  auto result = db(select(all_of(tab)).from(tab));
  std::cerr
      << "Accessing a field directly from the result (using the current row): "
      << result.begin()->int_n << std::endl;
  std::cerr << "Can do that again, no problem: " << result.begin()->int_n
            << std::endl;

  std::cerr << "--------------------------------------" << std::endl;
  auto tx = start_transaction(db);
  for (const auto& row :
       db(select(all_of(tab),
                 value(select(max(tab.int_n).as(something)).from(tab))
                     .as(something))
              .from(tab))) {
    const auto x = row.int_n;
    const auto a = row.something;
    std::cout << ">>>" << x << ", " << a << std::endl;
  }
  for (const auto& row :
       db(select(tab.int_n, tab.text_nn_d, tab.bool_n, trim(tab.text_nn_d).as(something))
              .from(tab))) {
    std::cerr << ">>> row.int_n: " << row.int_n << ", row.text_nn_d: " << row.text_nn_d
              << ", row.bool_n: " << row.bool_n << ", row.something: '"
              << row.something << "'" << std::endl;
    // check something
    assert(string_util::trim(std::string(row.text_nn_d)) == row.something);
    // end
  };

  for (const auto& row :
       db(select(all_of(tab),
                 value(select(trim(tab.text_nn_d).as(something)).from(tab))
                     .as(something))
              .from(tab))) {
    const std::optional<int64_t> x = row.int_n;
    const std::optional<std::string_view> a = row.something;
    std::cout << ">>>" << x << ", " << a << std::endl;
  }

  tx.commit();
  std::cerr << "--------------------------------------" << std::endl;

  return 0;
}
