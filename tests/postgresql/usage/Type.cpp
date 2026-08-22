/**
 * Copyright © 2017 Volker Aßmann
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

int Type(int, char*[]) {
  sql::connection db = sql::make_test_connection();

  try {
    test::createtab_foo(db);
    test::createtab_bar(db);

    const auto tab = test::tab_bar{};
    db(insert_into(tab).default_values());
    for (const auto& row : db(select(all_of(tab)).from(tab))) {
      require_equal(__LINE__, row.int_n.has_value(), false);
      require_equal(__LINE__, row.text_n.has_value(), false);
      require_equal(__LINE__, row.bool_nn, false);
    }

    db(update(tab).set(tab.int_n = 10, tab.text_n = "Cookies!",
                       tab.bool_nn = true));

    for (const auto& row : db(select(all_of(tab)).from(tab))) {
      require_equal(__LINE__, row.int_n.has_value(), true);
      require_equal(__LINE__, row.int_n.value(), 10);
      require_equal(__LINE__, row.text_n.has_value(), true);
      require_equal(__LINE__, row.text_n.value(), "Cookies!");
      require_equal(__LINE__, row.bool_nn, true);
    }

    db(update(tab).set(tab.int_n = 20, tab.text_n = "Monster",
                       tab.bool_nn = false));

    for (const auto& row : db(select(all_of(tab)).from(tab))) {
      require_equal(__LINE__, row.int_n.value(), 20);
      require_equal(__LINE__, row.text_n.value(), "Monster");
      require_equal(__LINE__, row.bool_nn, false);
    }

    auto prepared_update = db.prepare(update(tab).set(
        tab.int_n = parameter(tab.int_n), tab.text_n = parameter(tab.text_n),
        tab.bool_nn = parameter(tab.bool_nn)));
    prepared_update.parameters.int_n = 30;
    prepared_update.parameters.text_n = "IceCream";
    prepared_update.parameters.bool_nn = true;
    std::cout << "---- running prepared update ----" << std::endl;
    db(prepared_update);
    std::cout << "---- finished prepared update ----" << std::endl;

    for (const auto& row : db(select(all_of(tab)).from(tab))) {
      require_equal(__LINE__, row.int_n.value(), 30);
      require_equal(__LINE__, row.text_n.value(), "IceCream");
      require_equal(__LINE__, row.bool_nn, true);
    }
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Unknown exception" << std::endl;
    return 1;
  }
  return 0;
}
