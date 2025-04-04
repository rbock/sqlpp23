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
#include "Tables.h"

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
  test::createTabSample(db);
  test::createTabFoo(db);

  const auto tab = test::TabSample{};

  // clear the table
  db(delete_from(tab));

  // explicit all_of(tab)
  for (const auto& row : db(select(all_of(tab)).from(tab))) {
    std::cerr << "row.alpha: " << row.alpha << ", row.beta: " << row.beta
              << ", row.gamma: " << row.gamma << std::endl;
  };
  std::cerr << __FILE__ << ": " << __LINE__ << std::endl;
  // selecting a table implicitly expands to all_of(tab)
  for (const auto& row : db(select(all_of(tab)).from(tab))) {
    std::cerr << "row.alpha: " << row.alpha << ", row.beta: " << row.beta
              << ", row.gamma: " << row.gamma << std::endl;
  };
  // insert
  std::cerr << "no of required columns: "
            << sqlpp::required_insert_columns_of_t<test::TabSample>::size()
            << std::endl;
  db(insert_into(tab).default_values());
  std::cout << "Last Insert ID: " << db.last_insert_id() << "\n";
  db(insert_into(tab).set(tab.gamma = true, dynamic(true, tab.alpha = 7)));
  db(insert_into(tab).set(tab.gamma = true, dynamic(false, tab.alpha = 7)));
  std::cout << "Last Insert ID: " << db.last_insert_id() << "\n";

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

  auto tx = start_transaction(db);
  test::TabFoo foo;
  for (const auto& row :
       db(select(all_of(tab), value(select(max(foo.omega).as(something))
                                        .from(foo)
                                        .where(foo.omega > tab.alpha))
                                  .as(something))
              .from(tab))) {
    std::optional<int64_t> x = row.alpha;
    std::optional<int64_t> a = row.something;
    std::cout << x << ", " << a << std::endl;
  }
  tx.commit();

  for (const auto& row :
       db(select(tab.alpha).from(tab.join(foo).on(tab.alpha == foo.omega)))) {
    std::cerr << row.alpha << std::endl;
  }

  for (const auto& row : db(select(tab.alpha).from(
           tab.left_outer_join(foo).on(tab.alpha == foo.omega)))) {
    std::cerr << row.alpha << std::endl;
  }

  auto ps = db.prepare(select(all_of(tab))
                           .from(tab)
                           .where(tab.alpha != parameter(tab.alpha) and
                                  tab.beta != parameter(tab.beta) and
                                  tab.gamma != parameter(tab.gamma)));
  ps.params.alpha = 7;
  ps.params.beta = "wurzelbrunft";
  ps.params.gamma = true;
  for (const auto& row : db(ps)) {
    std::cerr << "bound result: alpha: " << row.alpha << std::endl;
    std::cerr << "bound result: beta: " << row.beta << std::endl;
    std::cerr << "bound result: gamma: " << row.gamma << std::endl;
  }

  std::cerr << "--------" << std::endl;
  const auto last_id =
      db(select(sqlpp::verbatim<sqlpp::integral>("last_insert_rowid()")
                    .as(something)))
          .front()
          .something;
  ps.params.alpha = last_id.value();
  ps.params.gamma = false;
  for (const auto& row : db(ps)) {
    std::cerr << "bound result: alpha: " << row.alpha << std::endl;
    std::cerr << "bound result: beta: " << row.beta << std::endl;
    std::cerr << "bound result: gamma: " << row.gamma << std::endl;
  }

  std::cerr << "--------" << std::endl;
  ps.params.beta = "kaesekuchen";
  for (const auto& row : db(ps)) {
    std::cerr << "bound result: alpha: " << row.alpha << std::endl;
    std::cerr << "bound result: beta: " << row.beta << std::endl;
    std::cerr << "bound result: gamma: " << row.gamma << std::endl;
  }

  auto pi = db.prepare(
      insert_into(tab).set(tab.beta = parameter(tab.beta), tab.gamma = true));
  pi.params.beta = "prepared cake";
  std::cerr << "Inserted: " << db(pi) << std::endl;

  auto pu = db.prepare(update(tab)
                           .set(tab.gamma = parameter(tab.gamma))
                           .where(tab.beta == "prepared cake"));
  pu.params.gamma = false;
  std::cerr << "Updated: " << db(pu) << std::endl;

  auto pr = db.prepare(delete_from(tab).where(tab.beta != parameter(tab.beta)));
  pr.params.beta = "prepared cake";
  std::cerr << "Deleted lines: " << db(pr) << std::endl;

  // Check that a prepared select is default-constructible
  {
    auto s = select(all_of(tab))
                 .from(tab)
                 .where((tab.beta.like(parameter(tab.beta)) and
                         tab.alpha == parameter(tab.alpha)) or
                        tab.gamma != parameter(tab.gamma));
    using P = decltype(db.prepare(s));
    P p;  // You must not use this one yet!
    p = db.prepare(s);
  }

  {
    // insert_or with static assignments
    auto i = db(sqlpp::sqlite3::insert_or_replace_into(tab).set(
        tab.beta = "test", tab.gamma = true));
    std::cerr << i << std::endl;

    i = db(sqlpp::sqlite3::insert_or_ignore_into(tab).set(tab.beta = "test",
                                                          tab.gamma = true));
    std::cerr << i << std::endl;
  }

  {
    // insert_or with a dynamic assignment
    auto i = db(sqlpp::sqlite3::insert_or_replace_into(tab).set(
        tab.beta = "test", dynamic(true, tab.gamma = true)));
    std::cerr << i << std::endl;

    i = db(sqlpp::sqlite3::insert_or_ignore_into(tab).set(
        tab.beta = "test", dynamic(true, tab.gamma = true)));
    std::cerr << i << std::endl;
  }

  assert(db(select(count(tab.id).as(something)).from(tab)).begin()->something);
  assert(db(select(all_of(tab))
                .from(tab)
                .where(tab.alpha.not_in(select(tab.alpha).from(tab))))
             .empty());

  auto x = sqlpp::statement_t<>{} << sqlpp::verbatim("PRAGMA user_version = 1");
  db(x);
  const int64_t pragmaValue =
      db(x << with_result_type_of(select(sqlpp::value(1).as(pragma))))
          .front()
          .pragma;
  std::cerr << pragmaValue << std::endl;

  // Testing sub select tables and unconditional joins
  const auto subQuery = select(tab.alpha).from(tab).as(sub);
  for (const auto& row : db(select(subQuery.alpha).from(subQuery))) {
    std::cerr << row.alpha;
  }

  for (const auto& row :
       db(select(subQuery.alpha).from(tab.cross_join(subQuery)))) {
    std::cerr << "row.alpha: " << row.alpha << std::endl;
  }

  return 0;
}
