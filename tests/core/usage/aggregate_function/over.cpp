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

#include <sqlpp23/tests/core/all.h>

int main(int, char*[]) {
  try {
    const auto tab = test::TabFoo{};
    auto db = sqlpp::mock_db::make_test_connection();

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
      std::ignore = row.avg_;
      std::ignore = row.count_;
      std::ignore = row.max_;
      std::ignore = row.min_;
      std::ignore = row.sum_;
    }
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
