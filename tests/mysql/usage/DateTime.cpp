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

const auto library_raii = sqlpp::mysql::scoped_library_initializer_t{};

namespace {
template <typename L, typename R>
auto require_close(int line, const L& l, const R& r) -> void {
  if (std::chrono::abs(l - r) > std::chrono::seconds{1}) {
    std::cerr << line << ": abs(";
    std::cerr << sqlpp::to_sql_string(std::cerr, l);
    std::cerr << " - ";
    std::cerr << sqlpp::to_sql_string(std::cerr, r);
    std::cerr << ") > 1s\n";
    throw std::runtime_error("Unexpected result");
  }
}
}  // namespace

namespace sql = sqlpp::mysql;
int DateTime(int, char*[]) {
  sql::global_library_init();
  try {
    const auto now = std::chrono::floor<::std::chrono::milliseconds>(
        std::chrono::system_clock::now());
    const auto today = std::chrono::floor<std::chrono::days>(now);
    const auto yesterday = today - std::chrono::days{1};
    const auto current = now - today;

    auto db = sql::make_test_connection();
    db(R"(SET time_zone = '+00:00')");  // To force MySQL's CURRENT_TIMESTAMP
                                        // into the right timezone
    test::createtab_date_time(db);

    const auto tab = test::tab_date_time{};
    db(insert_into(tab).default_values());
    for (const auto& row : db(select(all_of(tab)).from(tab))) {
      require_equal(__LINE__, row.date_n.has_value(), false);
      require_equal(__LINE__, row.timestamp_n.has_value(), false);
      require_close(__LINE__, row.dateTimestampND.value(), now);
      require_equal(__LINE__, row.time_n.has_value(), false);
    }

    db(update(tab)
           .set(tab.date_n = today, tab.timestamp_n = now,
                tab.time_n = current)
           .where(true));

    for (const auto& row : db(select(all_of(tab)).from(tab).where(true))) {
      require_equal(__LINE__, row.date_n.value(), today);
      require_equal(__LINE__, row.timestamp_n.value(), now);
      require_close(__LINE__, row.time_n.value(), current);
    }

    auto statement = db.prepare(select(all_of(tab)).from(tab).where(true));
    for (const auto& row : db(statement)) {
      require_close(__LINE__, row.dateTimestampND.value(), now);
      require_equal(__LINE__, row.date_n.value(), today);
      require_equal(__LINE__, row.timestamp_n.value(), now);
      require_close(__LINE__, row.time_n.value(), current);
    }

    db(update(tab)
           .set(tab.date_n = yesterday, tab.timestamp_n = now)
           .where(true));

    for (const auto& row : db(select(all_of(tab)).from(tab).where(true))) {
      require_equal(__LINE__, row.date_n.value(), yesterday);
      require_equal(__LINE__, row.timestamp_n.value(), now);
    }

    auto prepared_update =
        db.prepare(update(tab)
                       .set(tab.date_n = parameter(tab.date_n),
                            tab.timestamp_n = parameter(tab.timestamp_n),
                            tab.time_n = parameter(tab.time_n))
                       .where(true));
    prepared_update.parameters.date_n = today;
    prepared_update.parameters.timestamp_n = now;
    prepared_update.parameters.time_n = current;
    std::cout << "---- running prepared update ----" << std::endl;
    db(prepared_update);
    std::cout << "---- finished prepared update ----" << std::endl;
    for (const auto& row : db(select(all_of(tab)).from(tab).where(true))) {
      require_equal(__LINE__, row.date_n.value(), today);
      require_equal(__LINE__, row.timestamp_n.value(), now);
      require_equal(__LINE__, row.time_n.value(), current);
    }
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Unkown exception" << std::endl;
  }
  return 0;
}
