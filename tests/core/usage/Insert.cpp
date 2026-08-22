/*
 * Copyright (c) 2013-2015, Roland Bock
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

#include <sqlpp26/tests/core/all.h>
#include <sqlpp26/tests/core/is_regular.h>

int Insert(int, char*[]) {
  sqlpp::mock_db::connection db = sqlpp::mock_db::make_test_connection();
  sqlpp::mock_db::context_t ctx;
  const auto maybe = true;
  const auto t = test::tab_bar{};
  const auto tabDateTime = test::tab_date_time{};
  const auto u = test::tab_foo{};

  {
    using T = decltype(insert_into(t));
    static_assert(sqlpp::is_regular<T>::value, "type requirement");
  }

  {
    using T = decltype(insert_into(t).set(t.text_n = "kirschauflauf"));
    static_assert(sqlpp::is_regular<T>::value, "type requirement");
  }

  db(insert_into(u).default_values());
  db(insert_into(t).set(t.bool_nn = true, t.text_n = "kirschauflauf"));
  db(insert_into(t).set(t.bool_nn = false, t.text_n = std::optional{"pie"},
                        t.int_n = std::nullopt));

  to_sql_string(ctx, insert_into(t).default_values());

  to_sql_string(ctx, insert_into(t));
  to_sql_string(ctx,
                insert_into(t).set(t.bool_nn = true, t.text_n = "kirschauflauf"));
  to_sql_string(ctx, insert_into(t).columns(t.bool_nn, t.text_n));
  auto multi_insert = insert_into(t).columns(t.bool_nn, t.text_n, t.int_n);
  multi_insert.add_values(t.bool_nn = true, t.text_n = "cheesecake", t.int_n = 1);
  multi_insert.add_values(t.bool_nn = false, t.text_n = sqlpp::default_value,
                          t.int_n = sqlpp::default_value);
  multi_insert.add_values(t.bool_nn = false, t.text_n = sqlpp::default_value,
                          dynamic(maybe, t.int_n = 7));
  multi_insert.add_values(t.bool_nn = true, t.text_n = std::optional{"pie"},
                          t.int_n = std::nullopt);
  std::cerr << to_sql_string(ctx, multi_insert) << std::endl;

  // Beware, you need exact types for inserted values in multi_insert
  insert_into(tabDateTime)
      .set(tabDateTime.timestamp_n = std::chrono::system_clock::now());

  auto multi_time_insert =
      insert_into(tabDateTime).columns(tabDateTime.timestamp_n);
  multi_time_insert.add_values(
      tabDateTime.timestamp_n =
          std::chrono::time_point_cast<std::chrono::microseconds>(
              std::chrono::system_clock::now()));

  db(multi_insert);

  db(insert_into(t).set(t.bool_nn = true,
                        t.int_n = sqlpp::verbatim<sqlpp::integral>("17+4")));
  db(insert_into(t).set(t.bool_nn = true, t.int_n = std::nullopt));
  db(insert_into(t).set(t.bool_nn = true, t.int_n = sqlpp::default_value));
  db(insert_into(t).set(t.bool_nn = true, t.int_n = 0));
  db(insert_into(t).set(t.bool_nn = true, dynamic(maybe, t.int_n = 0)));

  db(insert_into(t).set(t.bool_nn = true, t.int_n = 0,
                        t.text_n = select(u.text_nn_d).from(u)));

  auto prepared_insert = db.prepare(insert_into(t).set(
      t.bool_nn = parameter(t.bool_nn), t.int_n = parameter(t.int_n)));
  prepared_insert.parameters.bool_nn = true;
  prepared_insert.parameters.int_n = std::nullopt;
  prepared_insert.parameters.int_n = 17;
  prepared_insert.parameters.int_n = std::nullopt;
  prepared_insert.parameters.int_n = std::optional{17};
  db(prepared_insert);

  auto prepared_insert_sv = db.prepare(insert_into(t).set(
      t.bool_nn = parameter(t.bool_nn), t.int_n = parameter(t.int_n),
      t.text_n = parameter(t.text_n)));
  prepared_insert_sv.parameters.bool_nn = true;
  prepared_insert_sv.parameters.int_n = 17;
  prepared_insert_sv.parameters.text_n = std::string_view("string_view");
  ;
  db(prepared_insert_sv);

  return 0;
}
