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

namespace {
SQLPP_CREATE_NAME_TAG(pragma);
SQLPP_CREATE_NAME_TAG(sub);
SQLPP_CREATE_NAME_TAG(something);
}  // namespace

namespace sql = sqlpp::sqlite3;
int Sample(int, char*[]) {
  auto db = sql::make_test_connection();
  test::createtab_foo(db);
  test::createtab_bar(db);

  const auto tab = test::tab_foo{};

  // clear the table
  db(delete_from(tab));

  // explicit all_of(tab)
  for (const auto& row : db(select(all_of(tab)).from(tab))) {
    std::cerr << "row.int_n: " << row.int_n << ", row.text_nn_d: " << row.text_nn_d
              << ", row.bool_n: " << row.bool_n << std::endl;
  };
  std::cerr << __FILE__ << ": " << __LINE__ << std::endl;
  // selecting a table implicitly expands to all_of(tab)
  for (const auto& row : db(select(all_of(tab)).from(tab))) {
    std::cerr << "row.int_n: " << row.int_n << ", row.text_nn_d: " << row.text_nn_d
              << ", row.bool_n: " << row.bool_n << std::endl;
  };
  // insert
  std::cerr << "no of required columns: "
            << sqlpp::required_insert_columns_of_t<test::tab_foo>::size()
            << std::endl;
  db(insert_into(tab).default_values());
  std::cout << "Last Insert ID: " << db.last_insert_id() << "\n";
  db(insert_into(tab).set(tab.bool_n = true, dynamic(true, tab.int_n = 7)));
  db(insert_into(tab).set(tab.bool_n = true, dynamic(false, tab.int_n = 7)));
  std::cout << "Last Insert ID: " << db.last_insert_id() << "\n";

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

  auto tx = start_transaction(db);
  test::tab_bar bar;
  for (const auto& row :
       db(select(all_of(tab), value(select(max(bar.int_n).as(something))
                                        .from(bar)
                                        .where(bar.int_n > tab.int_n))
                                  .as(something))
              .from(tab))) {
    std::optional<int64_t> x = row.int_n;
    std::optional<int64_t> a = row.something;
    std::cout << x << ", " << a << std::endl;
  }
  tx.commit();

  for (const auto& row :
       db(select(tab.int_n).from(tab.join(bar).on(tab.int_n == bar.int_n)))) {
    std::cerr << row.int_n << std::endl;
  }

  for (const auto& row : db(select(tab.int_n).from(
           tab.left_outer_join(bar).on(tab.int_n == bar.int_n)))) {
    std::cerr << row.int_n << std::endl;
  }

  auto ps = db.prepare(select(all_of(tab))
                           .from(tab)
                           .where(tab.int_n != parameter(tab.int_n) and
                                  tab.text_nn_d != parameter(tab.text_nn_d) and
                                  tab.bool_n != parameter(tab.bool_n)));
  ps.parameters.int_n = 7;
  ps.parameters.text_nn_d = "wurzelbrunft";
  ps.parameters.bool_n = true;
  for (const auto& row : db(ps)) {
    std::cerr << "bound result: int_n: " << row.int_n << std::endl;
    std::cerr << "bound result: text_nn_d: " << row.text_nn_d << std::endl;
    std::cerr << "bound result: bool_n: " << row.bool_n << std::endl;
  }

  std::cerr << "--------" << std::endl;
  const auto last_id =
      db(select(sqlpp::verbatim<sqlpp::integral>("last_insert_rowid()")
                    .as(something)))
          .front()
          .something;
  ps.parameters.int_n = last_id.value();
  ps.parameters.bool_n = false;
  for (const auto& row : db(ps)) {
    std::cerr << "bound result: int_n: " << row.int_n << std::endl;
    std::cerr << "bound result: text_nn_d: " << row.text_nn_d << std::endl;
    std::cerr << "bound result: bool_n: " << row.bool_n << std::endl;
  }

  std::cerr << "--------" << std::endl;
  ps.parameters.text_nn_d = "kaesekuchen";
  for (const auto& row : db(ps)) {
    std::cerr << "bound result: int_n: " << row.int_n << std::endl;
    std::cerr << "bound result: text_nn_d: " << row.text_nn_d << std::endl;
    std::cerr << "bound result: bool_n: " << row.bool_n << std::endl;
  }

  auto pi = db.prepare(
      insert_into(tab).set(tab.text_nn_d = parameter(tab.text_nn_d), tab.bool_n = true));
  pi.parameters.text_nn_d = "prepared cake";
  std::cerr << "Inserted: " << db(pi).last_insert_id << std::endl;

  auto pu = db.prepare(update(tab)
                           .set(tab.bool_n = parameter(tab.bool_n))
                           .where(tab.text_nn_d == "prepared cake"));
  pu.parameters.bool_n = false;
  std::cerr << "Updated: " << db(pu).last_insert_id << std::endl;

  auto pr = db.prepare(delete_from(tab).where(tab.text_nn_d != parameter(tab.text_nn_d)));
  pr.parameters.text_nn_d = "prepared cake";
  std::cerr << "Deleted lines: " << db(pr).affected_rows << std::endl;

  {
    // insert_or with static assignments
    auto i = db(sqlpp::sqlite3::insert_or_replace().into(tab).set(
        tab.text_nn_d = "test", tab.bool_n = true)).affected_rows;
    std::cerr << i << std::endl;

    i = db(sqlpp::sqlite3::insert_or_ignore().into(tab).set(tab.text_nn_d = "test",
                                                          tab.bool_n = true)).affected_rows;
    std::cerr << i << std::endl;
  }

  {
    // insert_or with a dynamic assignment
    auto i = db(sqlpp::sqlite3::insert_or_replace().into(tab).set(
        tab.text_nn_d = "test", dynamic(true, tab.bool_n = true))).last_insert_id;
    std::cerr << i << std::endl;

    i = db(sqlpp::sqlite3::insert_or_ignore().into(tab).set(
        tab.text_nn_d = "test", dynamic(true, tab.bool_n = true))).last_insert_id;
    std::cerr << i << std::endl;
  }

  assert(db(select(count(tab.id).as(something)).from(tab)).begin()->something);
  assert(db(select(all_of(tab))
                .from(tab)
                .where(tab.int_n.not_in(select(tab.int_n).from(tab))))
             .empty());

  auto x = sqlpp::statement_t<>{}
           << sqlpp::verbatim_clause("PRAGMA user_version = 1");
  db(x);
  const int64_t pragmaValue =
      db(x << with_result_type_of(select(sqlpp::value(1).as(pragma))))
          .front()
          .pragma;
  std::cerr << pragmaValue << std::endl;

  // Testing sub select tables and unconditional joins
  const auto subQuery = select(tab.int_n).from(tab).as(sub);
  for (const auto& row : db(select(subQuery.int_n).from(subQuery))) {
    std::cerr << row.int_n;
  }

  for (const auto& row :
       db(select(subQuery.int_n).from(tab.cross_join(subQuery)))) {
    std::cerr << "row.int_n: " << row.int_n << std::endl;
  }

  return 0;
}
