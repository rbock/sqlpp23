/*
 * Copyright (c) 2013 - 2015, Roland Bock
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

#include <sqlpp26/tests/postgresql/all.h>

namespace {
const auto now = std::chrono::floor<::std::chrono::microseconds>(
    std::chrono::system_clock::now());
const auto today = std::chrono::floor<std::chrono::days>(now);
const auto yesterday = today - std::chrono::days{1};
const auto current = now - today;

}  // namespace

int DateTime(int, char*[]) {
  namespace sql = sqlpp::postgresql;

  sql::connection db = sql::make_test_connection();

  test::createtab_date_time(db);

  test::tab_date_time tab = {};
  try {
    db(insert_into(tab).default_values());
    for (const auto& row : db(select(all_of(tab)).from(tab))) {
      require_equal(__LINE__, row.date_n.has_value(), false);
      require_equal(__LINE__, row.time_nTz.has_value(), false);
      require_equal(__LINE__, row.timestamp_nTz.has_value(), false);
    }

    db(update(tab).set(tab.date_n = today, tab.time_nTz = current,
                       tab.timestamp_nTz = now));

    for (const auto& row : db(select(all_of(tab)).from(tab))) {
      require_equal(__LINE__, row.date_n.value(), today);
      require_equal(__LINE__, row.time_nTz.value(), current);
      require_equal(__LINE__, row.timestamp_nTz.value(), now);
    }

    db(update(tab).set(tab.date_n = yesterday, tab.timestamp_nTz = now));

    for (const auto& row : db(select(all_of(tab)).from(tab))) {
      require_equal(__LINE__, row.date_n.value(), yesterday);
      require_equal(__LINE__, row.time_nTz.value(), current);
      require_equal(__LINE__, row.timestamp_nTz.value(), now);
    }

    auto prepared_update = db.prepare(
        update(tab).set(tab.date_n = parameter(tab.date_n),
                        tab.time_nTz = parameter(tab.time_nTz),
                        tab.timestamp_nTz = parameter(tab.timestamp_nTz)));
    prepared_update.parameters.date_n = today;
    prepared_update.parameters.time_nTz = current;
    prepared_update.parameters.timestamp_nTz = now;
    std::cout << "---- running prepared update ----" << std::endl;
    db(prepared_update);
    std::cout << "---- finished prepared update ----" << std::endl;
    for (const auto& row : db(select(all_of(tab)).from(tab))) {
      require_equal(__LINE__, row.date_n.value(), today);
      require_equal(__LINE__, row.time_nTz.value(), current);
      require_equal(__LINE__, row.timestamp_nTz.value(), now);
    }
  } catch (const sqlpp::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
