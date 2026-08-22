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

#include <utility>

#include <sqlpp26/tests/mysql/all.h>

namespace {
const auto library_raii =
    sqlpp::mysql::scoped_library_initializer_t{0, nullptr, nullptr};

namespace sql = sqlpp::mysql;
const auto tab = test::tab_foo{};

SQLPP_CREATE_NAME_TAG(something);
SQLPP_CREATE_NAME_TAG(max_int_n);
}  // namespace

template <typename... Names, typename... Values, size_t... Idx>
void printRowWithNamesImpl(const std::tuple<Names...>& names,
                           const std::tuple<Values...>& values,
                           std::index_sequence<Idx...>) {
  ((std::cerr << std::get<Idx>(names) << ": " << std::get<Idx>(values) << " "),
   ...);
  std::cerr << "\n";
}

template <typename... Names, typename... Values>
void printRowWithNames(const std::tuple<Names...>& names,
                       const std::tuple<Values...>& values) {
  printRowWithNamesImpl(names, values,
                        std::make_index_sequence<sizeof...(Values)>());
}

void testSelectAll(sql::connection& db, int expectedRowCount) {
  std::cerr << "--------------------------------------" << std::endl;
  int i = 0;
  for (const auto& row : db(sqlpp::select(all_of(tab)).from(tab))) {
    ++i;
    printRowWithNames(get_sql_name_tuple(row), as_tuple(row));
    std::cerr << ">>> row.id: " << row.id << ", >>> row.int_n: " << row.int_n
              << ", row.text_nn_d: " << row.text_nn_d << ", row.bool_n: " << row.bool_n
              << std::endl;
    assert(i == row.id);
  };
  assert(i == expectedRowCount);

  auto preparedSelectAll = db.prepare(sqlpp::select(all_of(tab)).from(tab));
  std::cerr << "--------------------------------------" << std::endl;
  i = 0;
  for (const auto& row : db(preparedSelectAll)) {
    ++i;
    std::cerr << ">>> row.id: " << row.id << ", >>> row.int_n: " << row.int_n
              << ", row.text_nn_d: " << row.text_nn_d << ", row.bool_n: " << row.bool_n
              << std::endl;
    assert(i == row.id);
  };
  assert(i == expectedRowCount);

  // Try running the same prepared statement again
  std::cerr << "--------------------------------------" << std::endl;
  i = 0;
  for (const auto& row : db(preparedSelectAll)) {
    ++i;
    std::cerr << ">>> row.id: " << row.id << ", >>> row.int_n: " << row.int_n
              << ", row.text_nn_d: " << row.text_nn_d << ", row.bool_n: " << row.bool_n
              << std::endl;
    assert(i == row.id);
  };
  assert(i == expectedRowCount);
  std::cerr << "--------------------------------------" << std::endl;
}

int Select(int, char*[]) {
  try {
    auto db = sql::make_test_connection();
    test::createtab_foo(db);

    testSelectAll(db, 0);
    db(insert_into(tab).default_values());
    testSelectAll(db, 1);
    db(insert_into(tab).set(tab.bool_n = true, tab.text_nn_d = "cheesecake"));
    testSelectAll(db, 2);
    db(insert_into(tab).set(tab.bool_n = true, tab.text_nn_d = "cheesecake"));
    testSelectAll(db, 3);

    db(select(coalesce(tab.text_nn_d, "fallback").as(something)).from(tab));

    // Test size functionality
    const auto test_size = db(select(all_of(tab)).from(tab));
    assert(test_size.size() == 3ull);

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
    db(select(all_of(tab))
           .from(tab)
           .where(tab.int_n ==
                  any(select(tab.int_n).from(tab).where(tab.int_n < 3))));

    db(select(all_of(tab)).from(tab).where(tab.int_n + tab.int_n > 3));
    db(select(all_of(tab)).from(tab).where((tab.text_nn_d + tab.text_nn_d) == ""));
    db(select(all_of(tab))
           .from(tab)
           .where((tab.text_nn_d + tab.text_nn_d).like("%'\"%")));

    // insert
    db(insert_into(tab).set(tab.bool_n = true));

    // update
    db(update(tab).set(tab.bool_n = false).where(tab.int_n.in(1)));
    db(update(tab)
           .set(tab.bool_n = false)
           .where(tab.int_n.in(std::vector<int>{1, 2, 3, 4})));

    // remove
    {
      db(delete_from(tab).where(tab.int_n == tab.int_n + 3));

      auto result = db(select(all_of(tab)).from(tab));
      std::cerr << "Accessing a field directly from the result (using the "
                   "current row): "
                << result.begin()->int_n << std::endl;
      std::cerr << "Can do that again, no problem: " << result.begin()->int_n
                << std::endl;
    }

    // transaction
    {
      auto tx = start_transaction(db);
      auto result =
          db(select(all_of(tab),
                    value(select(max(tab.int_n).as(max_int_n)).from(tab))
                        .as(max_int_n))
                 .from(tab));
      if (const auto& row = *result.begin()) {
        std::optional<int64_t> a = row.int_n;
        std::optional<int64_t> m = row.max_int_n;
        std::cerr << "-----------------------------" << a << ", " << m
                  << std::endl;
      }
      tx.commit();
    }
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
