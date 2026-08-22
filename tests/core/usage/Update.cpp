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

int Update(int, char*[]) {
  const auto maybe = true;
  sqlpp::mock_db::connection db = sqlpp::mock_db::make_test_connection();
  sqlpp::mock_db::context_t printer;

  const auto t = test::tab_bar{};

  {
    using T = decltype(update(t));
    static_assert(sqlpp::is_regular<T>::value, "type requirement");
  }

  {
    using T = decltype(update(t)
                           .set(t.bool_nn = false)
                           .where(t.text_n != "transparent"));
    static_assert(sqlpp::is_regular<T>::value, "type requirement");
  }

  to_sql_string(printer, update(t));
  to_sql_string(printer, update(t).set(t.bool_nn = false));
  to_sql_string(
      printer, update(t).set(t.bool_nn = false).where(t.text_n != "transparent"));
  to_sql_string(printer, update(t)
                             .set(t.text_n = "opaque")
                             .where(t.text_n != t.text_n + "this is nonsense"));
  to_sql_string(printer, update(t)
                             .set(t.text_n = "opaque")
                             .where(t.text_n != t.text_n + "this is nonsense"));

  to_sql_string(
      printer,
      update(t)
          .set(dynamic(maybe, t.text_n = "opaque"))
          .where(dynamic(maybe, t.text_n != t.text_n + "this is nonsense")));

  db(update(t).set(t.int_n = sqlpp::verbatim<sqlpp::integral>("17+4")));
  db(update(t)
         .set(t.int_n = sqlpp::verbatim<sqlpp::integral>("17+4"))
         .where(sqlpp::verbatim<sqlpp::text>("'hansi'") == "hansi"));
  db(update(t).set(t.int_n = std::nullopt));
  db(update(t).set(t.int_n = sqlpp::default_value));

  db(update(t).set(t.int_n = t.id * 2, t.text_n = t.text_n + " and cake"));
  db(update(t).set(
      t.int_n = t.id * 2,
      maybe ? dynamic(t.text_n = t.text_n + " and cake") : std::nullopt));

  return 0;
}
