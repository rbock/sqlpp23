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

int Union(int, char*[]) {
  sqlpp::mock_db::connection db = sqlpp::mock_db::make_test_connection();

  const auto t = test::tab_bar{};
  const auto f = test::tab_foo{};

  db(select(f.int_n.as<"id">()).from(f).union_distinct(select(t.id).from(t)));
  db(select(f.int_n.as<"id">()).from(f).union_all(select(t.id).from(t)));

  // t.id can be null, a given value cannot
  db(select(t.id).from(t).union_all(select(sqlpp::value(1).as<"id">())));


  // t.text_n can be null, f.text_nn_d cannot
  static_assert(
      sqlpp::is_optional<sqlpp::data_type_of_t<decltype(t.text_n)>>::value, "");
  static_assert(not sqlpp::is_optional<
                    sqlpp::data_type_of_t<decltype(f.text_nn_d)>>::value,
                "");
  db(select(t.text_n).from(t).union_all(
      // Note the text_n. This can be done better with reflection.
      select(f.text_nn_d.as<"text_n">()).from(f)));

  auto u = select(f.int_n.as<"id">()).from(f)
               .union_all(select(t.id).from(t))
               .as<"u">();

  db(select(all_of(u)).from(u).union_all(select(t.int_n.as<"id">()).from(t)));
  db(select(u.id).from(u).union_all(select(t.int_n.as<"id">()).from(t)));

  db(select(t.id)
         .from(t)
         .union_all(select(t.id).from(t))
         .union_all(select(t.id).from(t)));

  return 0;
}
