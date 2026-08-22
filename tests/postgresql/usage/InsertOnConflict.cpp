/**
 * Copyright © 2014-2019, Matthijs Möhlmann
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
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

#include <sqlpp26/tests/postgresql/all.h>

namespace sql = sqlpp::postgresql;

int InsertOnConflict(int, char*[]) {
  test::tab_foo foo = {};

  sql::connection db = sql::make_test_connection();

  test::createtab_foo(db);

  // Test on conflict
  db(sql::insert_into(foo).default_values().on_conflict().do_nothing());

  // Test on conflict (with conflict target)
  db(sql::insert_into(foo).default_values().on_conflict(foo.id).do_nothing());

  // Test on conflict (with mulitple conflict targets)
  db(sql::insert_into(foo)
         .default_values()
         .on_conflict(foo.id, dynamic(true, foo.int_nnU))
         .do_nothing());
  db(sql::insert_into(foo)
         .default_values()
         .on_conflict(foo.id, dynamic(false, foo.int_nnU))
         .do_nothing());

  // Conflict target
  db(sql::insert_into(foo).default_values().on_conflict(foo.id).do_update(
      foo.int_n = 5, foo.text_nn_d = "test bla", foo.bool_n = true));

  // With where statement
  for (const auto& row : db(sql::insert_into(foo)
                                .default_values()
                                .on_conflict(foo.id)
                                .do_update(foo.int_n = 5,
          dynamic(true, foo.text_nn_d = "test bla"), foo.bool_n = true)
                                .where(foo.int_n == 2)
                                .returning(foo.text_nn_d))) {
    std::cout << row.text_nn_d << std::endl;
  }

  // Returning
  for (const auto& row : db(sql::insert_into(foo)
                                .default_values()
                                .on_conflict(foo.id)
                                .do_update(foo.int_n = 5,
          foo.text_nn_d = "test bla", foo.bool_n = true)
                                .returning(foo.int_n))) {
    std::cout << row.int_n << std::endl;
  }

  return 0;
}
