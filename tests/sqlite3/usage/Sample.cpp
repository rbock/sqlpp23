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

#include <cassert>
#include <iostream>
#include <vector>

#include <sqlpp23/sqlite3/sqlite3.h>
#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/tests/sqlite3/make_test_connection.h>
#include <sqlpp23/tests/sqlite3/tables.h>

#ifdef SQLPP_USE_SQLCIPHER
#include <sqlcipher/sqlite3.h>
#else
#include <sqlite3.h>
#endif

namespace {
SQLPP_CREATE_NAME_TAG(pragma);
SQLPP_CREATE_NAME_TAG(sub);
SQLPP_CREATE_NAME_TAG(something);

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::optional<T>& t) {
  if (not t)
    return os << "NULL";
  return os << t.value();
}
}  // namespace

namespace sql = sqlpp::sqlite3;
int Sample(int, char*[]) {
  auto db = sql::make_test_connection();
  test::createTabFoo(db);
  test::createTabBar(db);

  const auto tab = test::TabFoo{};

  // clear the table
  db(delete_from(tab));

  // explicit all_of(tab)
  for (const auto& row : db(select(all_of(tab)).from(tab))) {
    std::cerr << "row.intN: " << row.intN << ", row.textNnD: " << row.textNnD
              << ", row.boolN: " << row.boolN << std::endl;
  };
  std::cerr << __FILE__ << ": " << __LINE__ << std::endl;
  // selecting a table implicitly expands to all_of(tab)
  for (const auto& row : db(select(all_of(tab)).from(tab))) {
    std::cerr << "row.intN: " << row.intN << ", row.textNnD: " << row.textNnD
              << ", row.boolN: " << row.boolN << std::endl;
  };
  // insert
  std::cerr << "no of required columns: "
            << sqlpp::required_insert_columns_of_t<test::TabFoo>::size()
            << std::endl;
  db(insert_into(tab).default_values());
  std::cout << "Last Insert ID: " << db.last_insert_id() << "\n";
  db(insert_into(tab).set(tab.boolN = true, dynamic(true, tab.intN = 7)));
  db(insert_into(tab).set(tab.boolN = true, dynamic(false, tab.intN = 7)));
  std::cout << "Last Insert ID: " << db.last_insert_id() << "\n";

  // update
  db(update(tab).set(tab.boolN = false).where(tab.intN.in(1)));
  db(update(tab)
         .set(tab.boolN = false)
         .where(tab.intN.in(std::vector<int>{1, 2, 3, 4})));

  // delete
  db(delete_from(tab).where(tab.intN == tab.intN + 3));

  auto result = db(select(all_of(tab)).from(tab));
  std::cerr
      << "Accessing a field directly from the result (using the current row): "
      << result.begin()->intN << std::endl;
  std::cerr << "Can do that again, no problem: " << result.begin()->intN
            << std::endl;

  auto tx = start_transaction(db);
  test::TabBar bar;
  for (const auto& row :
       db(select(all_of(tab), value(select(max(bar.intN).as(something))
                                        .from(bar)
                                        .where(bar.intN > tab.intN))
                                  .as(something))
              .from(tab))) {
    std::optional<int64_t> x = row.intN;
    std::optional<int64_t> a = row.something;
    std::cout << x << ", " << a << std::endl;
  }
  tx.commit();

  for (const auto& row :
       db(select(tab.intN).from(tab.join(bar).on(tab.intN == bar.intN)))) {
    std::cerr << row.intN << std::endl;
  }

  for (const auto& row : db(select(tab.intN).from(
           tab.left_outer_join(bar).on(tab.intN == bar.intN)))) {
    std::cerr << row.intN << std::endl;
  }

  auto ps = db.prepare(select(all_of(tab))
                           .from(tab)
                           .where(tab.intN != parameter(tab.intN) and
                                  tab.textNnD != parameter(tab.textNnD) and
                                  tab.boolN != parameter(tab.boolN)));
  ps.params.intN = 7;
  ps.params.textNnD = "wurzelbrunft";
  ps.params.boolN = true;
  for (const auto& row : db(ps)) {
    std::cerr << "bound result: intN: " << row.intN << std::endl;
    std::cerr << "bound result: textNnD: " << row.textNnD << std::endl;
    std::cerr << "bound result: boolN: " << row.boolN << std::endl;
  }

  std::cerr << "--------" << std::endl;
  const auto last_id =
      db(select(sqlpp::verbatim<sqlpp::integral>("last_insert_rowid()")
                    .as(something)))
          .front()
          .something;
  ps.params.intN = last_id.value();
  ps.params.boolN = false;
  for (const auto& row : db(ps)) {
    std::cerr << "bound result: intN: " << row.intN << std::endl;
    std::cerr << "bound result: textNnD: " << row.textNnD << std::endl;
    std::cerr << "bound result: boolN: " << row.boolN << std::endl;
  }

  std::cerr << "--------" << std::endl;
  ps.params.textNnD = "kaesekuchen";
  for (const auto& row : db(ps)) {
    std::cerr << "bound result: intN: " << row.intN << std::endl;
    std::cerr << "bound result: textNnD: " << row.textNnD << std::endl;
    std::cerr << "bound result: boolN: " << row.boolN << std::endl;
  }

  auto pi = db.prepare(
      insert_into(tab).set(tab.textNnD = parameter(tab.textNnD), tab.boolN = true));
  pi.params.textNnD = "prepared cake";
  std::cerr << "Inserted: " << db(pi) << std::endl;

  auto pu = db.prepare(update(tab)
                           .set(tab.boolN = parameter(tab.boolN))
                           .where(tab.textNnD == "prepared cake"));
  pu.params.boolN = false;
  std::cerr << "Updated: " << db(pu) << std::endl;

  auto pr = db.prepare(delete_from(tab).where(tab.textNnD != parameter(tab.textNnD)));
  pr.params.textNnD = "prepared cake";
  std::cerr << "Deleted lines: " << db(pr) << std::endl;

  // Check that a prepared select is default-constructible
  {
    auto s = select(all_of(tab))
                 .from(tab)
                 .where((tab.textNnD.like(parameter(tab.textNnD)) and
                         tab.intN == parameter(tab.intN)) or
                        tab.boolN != parameter(tab.boolN));
    using P = decltype(db.prepare(s));
    P p;  // You must not use this one yet!
    p = db.prepare(s);
  }

  {
    // insert_or with static assignments
    auto i = db(sqlpp::sqlite3::insert_or_replace().into(tab).set(
        tab.textNnD = "test", tab.boolN = true));
    std::cerr << i << std::endl;

    i = db(sqlpp::sqlite3::insert_or_ignore().into(tab).set(tab.textNnD = "test",
                                                          tab.boolN = true));
    std::cerr << i << std::endl;
  }

  {
    // insert_or with a dynamic assignment
    auto i = db(sqlpp::sqlite3::insert_or_replace().into(tab).set(
        tab.textNnD = "test", dynamic(true, tab.boolN = true)));
    std::cerr << i << std::endl;

    i = db(sqlpp::sqlite3::insert_or_ignore().into(tab).set(
        tab.textNnD = "test", dynamic(true, tab.boolN = true)));
    std::cerr << i << std::endl;
  }

  assert(db(select(count(tab.id).as(something)).from(tab)).begin()->something);
  assert(db(select(all_of(tab))
                .from(tab)
                .where(tab.intN.not_in(select(tab.intN).from(tab))))
             .empty());

  auto x = sqlpp::statement_t<>{} << sqlpp::verbatim("PRAGMA user_version = 1");
  db(x);
  const int64_t pragmaValue =
      db(x << with_result_type_of(select(sqlpp::value(1).as(pragma))))
          .front()
          .pragma;
  std::cerr << pragmaValue << std::endl;

  // Testing sub select tables and unconditional joins
  const auto subQuery = select(tab.intN).from(tab).as(sub);
  for (const auto& row : db(select(subQuery.intN).from(subQuery))) {
    std::cerr << row.intN;
  }

  for (const auto& row :
       db(select(subQuery.intN).from(tab.cross_join(subQuery)))) {
    std::cerr << "row.intN: " << row.intN << std::endl;
  }

  return 0;
}
