/*
 * Copyright (c) 2013-2016, Roland Bock
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

#include <algorithm>

#include <sqlpp23/tests/core/all.h>

struct to_cerr {
  template <typename... Fields>
  auto operator()(const Fields&... fields) const -> void {
    (std::cerr << ... << fields);
  }
};

template <typename Row>
void print_row(Row const& row) {
  const std::optional<int64_t> a = row.id;
  const std::optional<std::string_view> b = row.textN;
  std::cout << a << ", " << b << std::endl;
}

SQLPP_CREATE_NAME_TAG(param2);
SQLPP_CREATE_NAME_TAG(cheese);
SQLPP_CREATE_NAME_TAG(average);
SQLPP_CREATE_NAME_TAG(N);

int Select(int, char*[]) {
  sqlpp::mock_db::connection db = sqlpp::mock_db::make_test_connection();
  sqlpp::mock_db::context_t printer;

  const auto maybe = true;
  const auto f = test::TabFoo{};
  const auto t = test::TabBar{};
  const auto tab_a = f.as(sqlpp::alias::a);

  select(count(t.id).as(N));
  select(sqlpp::count(1).as(N));
  select(count(sqlpp::value(1)).as(N));

  std::cerr << to_sql_string(printer,
                             select(sqlpp::value(false).as(sqlpp::alias::a)))
            << std::endl;
  for (const auto& row : db(select(sqlpp::value(false).as(sqlpp::alias::a)))) {
    std::cout << row.a << std::endl;
  }

  {
    // using stl algorithms
    auto rows = db(select(all_of(t)).from(t));
    // nicer in C++14
    std::for_each(rows.begin(), rows.end(),
                  &print_row<decltype(*rows.begin())>);
  }

  for (const auto& row : db(select(all_of(t)).from(t))) {
    const std::optional<int64_t> a = row.id;
    const std::optional<std::string_view> b = row.textN;
    std::cout << a << ", " << b << std::endl;
  }

  for (const auto& row : db(select(all_of(t), t.boolNn.as(t))
                                .from(t)
                                .where(t.id > 7 and trim(t.textN) == "test")
                                .for_update())) {
    const std::optional<int64_t> a = row.id;
    const std::optional<std::string_view> b = row.textN;
    const bool g = row.tabBar;
    std::cout << a << ", " << b << ", " << g << std::endl;
  }


  for (const auto& row :
       db(select(all_of(t), f.textNnD)
              .from(t.join(f).on(t.id > f.doubleN and not t.boolNn)))) {
    std::cout << row.id << std::endl;
  }

  for (const auto& row : db(select(all_of(t), f.textNnD)
                                .from(t.join(f)
                                          .on(t.id > f.doubleN)
                                          .join(tab_a)
                                          .on(t.id == tab_a.doubleN)))) {
    std::cout << row.id << std::endl;
  }

  for (const auto& row :
       db(select(sqlpp::count(1).as(N), avg(t.id).as(average)).from(t))) {
    std::cout << row.N << std::endl;
  }

  for (const auto& row : db(select(count(t.id).as(N), avg(t.id).as(average))
                                .from(t)
                                .where(t.id == 0))) {
    std::cout << row.N << std::endl;
  }

  auto stat = sqlpp::select()
                  .columns(sqlpp::all, all_of(t))
                  .from(t)
                  .where(t.id > 0)
                  .group_by(t.id)
                  .order_by(t.boolNn.asc())
                  .having(max(t.boolNn) > 0)
                  .offset(19u)
                  .limit(7u);
  std::cerr << to_sql_string(printer, stat) << std::endl;

  auto s = sqlpp::select()
               .columns(sqlpp::distinct, t.id)
               .from(t)
               .where(t.id > 3)
               .group_by(t.id)
               .order_by(t.id.asc())
               .having(sum(t.id) > parameter(t.intN))
               .limit(32u)
               .offset(7u);
  for (const auto& row : db(db.prepare(s))) {
    const std::optional<int64_t> a = row.id;
    std::cout << a << std::endl;
  }

  auto s2 = sqlpp::select()
                .columns(dynamic(maybe, sqlpp::distinct), dynamic(maybe, t.id))
                .from(dynamic(maybe, t))
                .where(dynamic(maybe, t.id > 3))
                .group_by(dynamic(maybe, t.id))
                .order_by(dynamic(maybe, t.id.asc()))
                .having(dynamic(maybe, sum(t.id) > 27))
                .limit(sqlpp::dynamic(maybe, 32u))
                .offset(sqlpp::dynamic(maybe, 7u));
  for (const auto& row : db(db.prepare(s2))) {
    const std::optional<int64_t> a = row.id;
    std::cout << a << std::endl;
  }

  std::cerr << to_sql_string(printer, s) << std::endl;

  select(sqlpp::value(7).as(t.id));

  for (const auto& row : db(select(sqlpp::case_when(true)
                                       .then(t.textN)
                                       .else_(std::nullopt)
                                       .as(t.textN))
                                .from(t))) {
    std::cerr << row.textN << std::endl;
  }

  for (const auto& row : db(select(all_of(t)).from(t))) {
    std::apply(to_cerr{}, row.as_tuple());
  }

  {
    auto transaction =
        start_transaction(db, sqlpp::isolation_level::read_committed);
    if (db._mock_data._last_isolation_level !=
        sqlpp::isolation_level::read_committed) {
      std::cout
          << "Error: transaction isolation level does not match expected level"
          << std::endl;
    }
  }
  db.set_default_isolation_level(sqlpp::isolation_level::read_uncommitted);
  {
    auto transaction = start_transaction(db);
    if (db._mock_data._last_isolation_level !=
        sqlpp::isolation_level::read_uncommitted) {
      std::cout
          << "Error: transaction isolation level does not match default level"
          << std::endl;
    }
  }

  // Move to type tests?
  for (const auto& row :
       db(select(f.doubleN, value(select(count(t.id).as(N)).from(t)).as(cheese))
              .from(f))) {
    std::cout << row.doubleN << " " << row.cheese << std::endl;
  }

  // checking #584
  auto abs = db.prepare(select(t.id).from(t).where(
      sqlpp::parameterized_verbatim<sqlpp::unsigned_integral>(
          "ABS(field1 -", sqlpp::parameter(t.id), ")") <=
      sqlpp::parameter(sqlpp::unsigned_integral(), param2)));
  abs.parameters.id = 7;
  abs.parameters.param2 = 7;
  std::ignore = abs;

  return 0;
}
