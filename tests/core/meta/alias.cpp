/*
 * Copyright (c) 2026 Roland Bock
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
#if SQLPP_INCLUDE_REFLECTION == 1
  const auto foo = test::TabFoo{};
  const auto bar = test::TabBar{};

  SQLPP_COMPARE(avg(bar.id + 7).as<"my_average">(),
                "AVG(tab_bar.id + 7) AS my_average");

  const auto table_a = foo.as<"table_a">();
  const auto table_b = bar.as<"table_b">();

  SQLPP_COMPARE(sqlpp::select(table_a.id.as<"id_a">(), table_a.intN,
                              table_b.id.as<"id_b">(), table_b.textN)
                    .from(table_a.join(table_b).on(table_a.id == table_b.id)),
                "SELECT table_a.id AS id_a, table_a.int_n, table_b.id AS id_b, "
                "table_b.text_n "
                "FROM tab_foo AS table_a "
                "INNER JOIN tab_bar AS table_b "
                "ON table_a.id = table_b.id");

  sqlpp::mock_db::connection db = sqlpp::mock_db::make_test_connection();
  for (const auto& row :
       db(sqlpp::select(table_a.id.as<"id_a">()).from(table_a))) {
    std::cout << row.id_a << std::endl;
  }
#endif

  return 0;
}
