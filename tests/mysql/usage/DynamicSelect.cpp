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

namespace {
const auto library_raii = sqlpp::mysql::scoped_library_initializer_t{};

}  // namespace

namespace sql = sqlpp::mysql;
int DynamicSelect(int, char*[]) {
  sql::global_library_init();
  try {
    auto db = sql::make_test_connection();
    test::createtab_foo(db);

    const auto tab = test::tab_foo{};
    db(insert_into(tab).set(tab.bool_n = true));
    auto i = insert_into(tab).columns(tab.text_nn_d, tab.bool_n);
    i.add_values(tab.text_nn_d = "rhabarbertorte", tab.bool_n = false);
    i.add_values(tab.text_nn_d = "cheesecake", tab.bool_n = false);
    i.add_values(tab.text_nn_d = "kaesekuchen", tab.bool_n = true);
    db(i);

    auto s = select(tab.int_n, dynamic(false, tab.text_nn_d)).from(tab);

    for (const auto& row : db(s)) {
      std::cerr << "row.int_n: " << row.int_n << ", row.text_nn_d: " << row.text_nn_d
                << std::endl;
    };
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
