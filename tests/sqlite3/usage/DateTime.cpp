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

#include <sqlpp23/tests/sqlite3/all.h>

namespace {
const auto now = std::chrono::floor<::std::chrono::milliseconds>(
    std::chrono::system_clock::now());
const auto today = std::chrono::floor<std::chrono::days>(now);
const auto time_of_day = std::chrono::microseconds{now - today};
const auto yesterday = today - std::chrono::days{1};
}  // namespace

namespace sql = sqlpp::sqlite3;
int DateTime(int, char*[]) {
  try {
    auto db = sql::make_test_connection();
    test::createTabDateTime(db);

    const auto tab = test::TabDateTime{};
    db(insert_into(tab).default_values());

    for (const auto& row : db(select(all_of(tab)).from(tab))) {
      require_equal(__LINE__, row.dateN == std::nullopt, true);
      require_equal(__LINE__, row.timestampN == std::nullopt, true);
    }

    db(update(tab).set(tab.dateN = today, tab.timestampN = now));

    for (const auto& row : db(select(all_of(tab)).from(tab))) {
      require_equal(__LINE__, row.dateN.value(), today);
      require_equal(__LINE__, row.timestampN.value(), now);
    }

    db(update(tab).set(tab.dateN = yesterday, tab.timestampN = now));

    for (const auto& row : db(select(all_of(tab)).from(tab))) {
      require_equal(__LINE__, row.dateN.value(), yesterday);
      require_equal(__LINE__, row.timestampN.value(), now);
    }

    auto prepared_update =
        db.prepare(update(tab).set(tab.dateN = parameter(tab.dateN),
                                   tab.timestampN = parameter(tab.timestampN),
                                   tab.timeN = parameter(tab.timeN)));
    prepared_update.parameters.dateN = today;
    prepared_update.parameters.timestampN = now;
    prepared_update.parameters.timeN = time_of_day;
    std::cout << "---- running prepared update ----" << std::endl;
    db(prepared_update);
    std::cout << "---- finished prepared update ----" << std::endl;
    for (const auto& row : db(select(all_of(tab)).from(tab))) {
      require_equal(__LINE__, row.dateN.value(), today);
      require_equal(__LINE__, row.timestampN.value(), now);
      require_equal(__LINE__, row.timeN.value(), time_of_day);
    }
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Unknown exception: " << std::endl;
    return 1;
  }

  return 0;
}
