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

#include <sqlpp26/tests/mysql/all.h>

namespace sql = sqlpp::mysql;
int Sample(int, char*[]) {
  sql::global_library_init();
  try {
    auto db = sql::make_test_connection();

    test::createtab_bar(db);
    test::createtab_foo(db);

    assert(not db(select(sqlpp::value(false).as(sqlpp::alias::a))).front().a);

    const auto tab = test::tab_bar{};
    // clear the table
    db(delete_from(tab));

    // Several ways of ensuring that tab is empty
    assert(
        not db(select(exists(select(tab.int_n).from(tab)).as(::sqlpp::alias::a)))
                .front()
                .a);  // this is probably the fastest
    assert(not db(select(count(tab.int_n).as(sqlpp::alias::a)).from(tab))
                   .front()
                   .a);
    assert(db(select(tab.int_n).from(tab)).empty());

    // explicit all_of(tab)
    std::cerr << __FILE__ << ": " << __LINE__ << std::endl;
    select(all_of(tab)).from(tab);
    std::cerr << __FILE__ << ": " << __LINE__ << std::endl;
    db(select(all_of(tab)).from(tab));
    std::cerr << __FILE__ << ": " << __LINE__ << std::endl;
    for (const auto& row : db(select(all_of(tab)).from(tab))) {
      std::cerr << __FILE__ << ": " << __LINE__ << std::endl;
      std::cerr << "row.int_n: " << row.int_n << ", row.text_n: " << row.text_n
                << ", row.bool_nn: " << row.bool_nn << std::endl;
    };
    // insert
    db(insert_into(tab).default_values());
    const auto x = select(all_of(tab)).from(tab);
    auto y = db.prepare(x);
    for (const auto& row : db(db.prepare(select(all_of(tab)).from(tab)))) {
      std::cerr << "int_n: " << row.int_n << std::endl;
      std::cerr << "text_n: " << row.text_n << std::endl;
      std::cerr << "bool_nn: " << row.bool_nn << std::endl;
    }
    db(insert_into(tab).set(tab.text_n = "kaesekuchen", tab.bool_nn = true));
    db(insert_into(tab).set(tab.text_n = "kaesekuchen",
                            dynamic(true, tab.bool_nn = true)));
    db(insert_into(tab).default_values());
    db(insert_into(tab).set(tab.text_n = "", tab.bool_nn = true));

    // update
    db(update(tab)
           .set(tab.bool_nn = false)
           .where(tab.int_n.in(std::vector<int>{1, 2, 3, 4})));
    db(update(tab).set(tab.bool_nn = true).where(tab.int_n.in(1)));

    // remove
    {
      db(delete_from(tab).where(tab.int_n == tab.int_n + 3));

      std::cerr << "+++++++++++++++++++++++++++" << std::endl;
      for (const auto& row : db(select(all_of(tab)).from(tab))) {
        std::cerr << __LINE__ << " row.text_n: " << row.text_n << std::endl;
      }
      std::cerr << "+++++++++++++++++++++++++++" << std::endl;
      decltype(db(select(all_of(tab)).from(tab))) result;
      result = db(select(all_of(tab)).from(tab));
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
                    value(select(max(tab.int_n).as(sqlpp::alias::a)).from(tab))
                        .as(sqlpp::alias::a))
                 .from(tab));
      if (const auto& row = *result.begin()) {
        const int64_t a = row.int_n.value_or(0);
        const std::optional<int64_t> m = row.a;
        std::cerr << __LINE__ << " row.int_n: " << a << ", row.max: " << m
                  << std::endl;
      }
      tx.commit();
    }

    test::tab_foo foo;
    for (const auto& row :
         db(select(tab.int_n).from(tab.join(foo).on(tab.int_n == foo.int_n)))) {
      std::cerr << row.int_n << std::endl;
    }

    for (const auto& row : db(select(tab.int_n).from(
             tab.left_outer_join(foo).on(tab.int_n == foo.int_n)))) {
      std::cerr << row.int_n << std::endl;
    }

    auto ps = db.prepare(select(all_of(tab))
                             .from(tab)
                             .where(tab.int_n != parameter(tab.int_n) and
                                    tab.text_n != parameter(tab.text_n) and
                                    tab.bool_nn != parameter(tab.bool_nn)));
    ps.parameters.int_n = 7;
    ps.parameters.text_n = "wurzelbrunft";
    ps.parameters.bool_nn = true;
    for (const auto& row : db(ps)) {
      std::cerr << "bound result: int_n: " << row.int_n << std::endl;
      std::cerr << "bound result: text_n: " << row.text_n << std::endl;
      std::cerr << "bound result: bool_nn: " << row.bool_nn << std::endl;
    }

    std::cerr << "--------" << std::endl;
    ps.parameters.bool_nn = false;
    for (const auto& row : db(ps)) {
      std::cerr << "bound result: int_n: " << row.int_n << std::endl;
      std::cerr << "bound result: text_n: " << row.text_n << std::endl;
      std::cerr << "bound result: bool_nn: " << row.bool_nn << std::endl;
    }

    std::cerr << "--------" << std::endl;
    ps.parameters.text_n = "kaesekuchen";
    for (const auto& row : db(ps)) {
      std::cerr << "bound result: int_n: " << row.int_n << std::endl;
      std::cerr << "bound result: text_n: " << row.text_n << std::endl;
      std::cerr << "bound result: bool_nn: " << row.bool_nn << std::endl;
    }

    auto pi = db.prepare(insert_into(tab).set(tab.text_n = parameter(tab.text_n),
                                              tab.bool_nn = true));
    pi.parameters.text_n = "prepared cake";
    std::cerr << "Inserted: " << db(pi).last_insert_id << std::endl;

    auto pu = db.prepare(update(tab)
                             .set(tab.bool_nn = parameter(tab.bool_nn))
                             .where(tab.text_n == "prepared cake"));
    pu.parameters.bool_nn = false;
    std::cerr << "Updated: " << db(pu).last_insert_id << std::endl;

    auto pr =
        db.prepare(delete_from(tab).where(tab.text_n != parameter(tab.text_n)));
    pr.parameters.text_n = "prepared cake";
    std::cerr << "Deleted lines: " << db(pr).affected_rows << std::endl;

    for (const auto& row :
         db(select(case_when(tab.bool_nn).then(tab.int_n).else_(foo.int_n).as(
                       tab.int_n))
                .from(tab.cross_join(foo)))) {
      std::cerr << row.int_n << std::endl;
    }
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
