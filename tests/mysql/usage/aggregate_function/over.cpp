/*
 * Copyright (c) 2025, Roland Bock
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

#include <sqlpp23/tests/mysql/all.h>

auto require_close(int line, double l, double r) -> void
{
  if (std::abs(l - r) > 0.001) {
    std::cerr << line << ": abs(";
    std::cerr << sqlpp::to_sql_string(std::cerr, l);
    std::cerr << " - ";
    std::cerr << sqlpp::to_sql_string(std::cerr, r);
    std::cerr << ") > 0.001\n";
    throw std::runtime_error("Unexpected result");
  }
}

namespace sql = sqlpp::mysql;
int main(int, char*[]) {
  sql::global_library_init();
  try {
    const auto tab = test::TabFoo{};
    auto db = sql::make_test_connection();

    test::createTabFoo(db);

    // clear the table
    db(truncate(tab));

    // insert
    db(insert_into(tab).set(tab.intN = 7));
    db(insert_into(tab).set(tab.intN = 7));
    db(insert_into(tab).set(tab.intN = 9));

    // select aggregates with over()
    for (const auto& row : db(select(
            avg(tab.intN).over().as(sqlpp::alias::avg_),
            count(tab.intN).over().as(sqlpp::alias::count_),
            max(tab.intN).over().as(sqlpp::alias::max_),
            min(tab.intN).over().as(sqlpp::alias::min_),
            sum(tab.intN).over().as(sqlpp::alias::sum_)
            ).from(tab))) {
      require_close(__LINE__, row.avg_.value(), 7.666);
      assert(row.count_ == 3);
      assert(row.max_.value() == 9);
      assert(row.min_.value() == 7);
      assert(row.sum_.value() == 23);
    }
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
