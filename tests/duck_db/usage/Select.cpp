/*
 * Copyright (c) 2026
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

#include <sqlpp23/tests/duck_db/all.h>

int Select(int, char*[]) {
  auto db = sqlpp::duck_db::make_test_connection({});
  test::createTabFoo(db);

  const auto foo = test::TabFoo{};

  db(sqlpp::duck_db::insert_into(foo)
         .set(foo.id = 1, foo.textNnD = "one", foo.intN = 7, foo.boolN = true));
  db(sqlpp::duck_db::insert_into(foo)
         .set(foo.id = 2, foo.textNnD = "two", foo.intN = 8, foo.boolN = false));

  // all_of
  {
    size_t rows = 0;
    for (const auto& row : db(select(all_of(foo)).from(foo))) {
      std::ignore = row.id;
      std::ignore = row.textNnD;
      ++rows;
    }
    require_equal(rows, 2u);
  }

  // where
  {
    size_t rows = 0;
    for (const auto& row :
         db(select(foo.id).from(foo).where(foo.intN > 7))) {
      require_equal(row.id, 2);
      ++rows;
    }
    require_equal(rows, 1u);
  }

  // order_by / limit
  {
    const auto first = db(select(foo.id).from(foo).order_by(foo.id.desc()).limit(1));
    require_equal(first.front().id, 2);
  }

  // sub select
  {
    size_t rows = 0;
    for (const auto& row :
         db(select(foo.id).from(foo).where(foo.id.in(
             select(foo.id).from(foo).where(foo.intN == 7))))) {
      require_equal(row.id, 1);
      ++rows;
    }
    require_equal(rows, 1u);
  }

  // aggregate function
  {
    const auto maximum = db(select(max(foo.intN).as(sqlpp::alias::a)).from(foo));
    require_equal(maximum.front().a.value(), 8);
  }

  // join
  {
    const auto bar = test::TabBar{};
    test::createTabBar(db);
    db(sqlpp::duck_db::insert_into(bar).set(bar.id = 1, bar.boolNn = true, bar.intN = 7));

    size_t rows = 0;
    for (const auto& row : db(select(foo.id).from(
             foo.join(bar).on(foo.id == bar.id)))) {
      std::ignore = row.id;
      ++rows;
    }
    require_equal(rows, 1u);
  }

  // union
  {
    size_t rows = 0;
    for (const auto& row :
         db(select(foo.id).from(foo).where(foo.intN == 7).union_all(
                select(foo.id).from(foo).where(foo.intN == 8)))) {
      std::ignore = row.id;
      ++rows;
    }
    require_equal(rows, 2u);
  }

  // prepared select
  {
    auto ps = db.prepare(select(foo.id).from(foo).where(foo.intN ==
                                                         parameter(foo.intN)));
    ps.parameters.intN = 8;
    size_t rows = 0;
    for (const auto& row : db(ps)) {
      require_equal(row.id, 2);
      ++rows;
    }
    require_equal(rows, 1u);

    ps.parameters.intN = 7;
    rows = 0;
    for (const auto& row : db(ps)) {
      require_equal(row.id, 1);
      ++rows;
    }
    require_equal(rows, 1u);
  }

  return 0;
}
