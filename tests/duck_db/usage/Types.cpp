/*
 * Copyright (c) 2026
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

#include <cmath>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <vector>

#include <sqlpp23/tests/duck_db/all.h>

int Types(int, char*[]) {
  auto db = sqlpp::duck_db::make_test_connection({});
  test::createTabFoo(db);

  const auto foo = test::TabFoo{};

  const auto blob = std::vector<uint8_t>{0x00, 0x7f, 0x80, 0xff};
  const auto date = std::chrono::sys_days{std::chrono::year{2026} /
                                          std::chrono::April / 1};
  const auto time = std::chrono::microseconds{std::chrono::hours{1} +
                                              std::chrono::minutes{2} +
                                              std::chrono::seconds{3}};
  const auto timestamp = sqlpp::chrono::sys_microseconds{date} + time;

  db(sqlpp::duck_db::insert_into(foo).set(
      foo.id = 1, foo.textNnD = "types",
      foo.intN = std::numeric_limits<int64_t>::min(),
      foo.uIntN = std::numeric_limits<uint64_t>::max(), foo.doubleN = 0.5,
      foo.boolN = true, foo.blobN = blob));

  for (const auto& row : db(select(all_of(foo)).from(foo))) {
    require_equal(row.intN.value(), std::numeric_limits<int64_t>::min());
    require_equal(row.uIntN.value(), std::numeric_limits<uint64_t>::max());
    require_equal(row.doubleN.value(), 0.5);
    require_equal(row.boolN.value(), true);
    require_equal(std::string{row.textNnD}, std::string{"types"});

    const std::span<const uint8_t> read_blob = *row.blobN;
    require_equal(read_blob.size(), blob.size());
    for (size_t i = 0; i < blob.size(); ++i) {
      require_equal(static_cast<int>(read_blob[i]), static_cast<int>(blob[i]));
    }
  }

  // NULL values surface as empty optionals.
  db(sqlpp::duck_db::insert_into(foo).set(
      foo.id = 2, foo.textNnD = "nulls", foo.intN = std::nullopt,
      foo.uIntN = std::nullopt, foo.doubleN = std::nullopt,
      foo.boolN = std::nullopt, foo.blobN = std::nullopt));

  for (const auto& row : db(select(all_of(foo)).from(foo).where(foo.id == 2))) {
    require_equal(row.intN.has_value(), false);
    require_equal(row.uIntN.has_value(), false);
    require_equal(row.doubleN.has_value(), false);
    require_equal(row.boolN.has_value(), false);
    require_equal(row.blobN.has_value(), false);
  }

  // NaN and infinity round trip.
  db(sqlpp::duck_db::update(foo)
         .set(foo.doubleN = std::numeric_limits<double>::quiet_NaN())
         .where(foo.id == 2));
  for (const auto& row : db(select(foo.doubleN).from(foo).where(foo.id == 2))) {
    require_equal(std::isnan(row.doubleN.value()), true);
  }

  // Date and time types.
  test::createTabDateTime(db);
  const auto tab = test::TabDateTime{};
  db(sqlpp::duck_db::insert_into(tab).set(tab.id = 1, tab.dateN = date,
                                          tab.timeN = time,
                                          tab.timestampN = timestamp));
  for (const auto& row : db(select(all_of(tab)).from(tab))) {
    require_equal(row.dateN.value(), date);
    require_equal(row.timeN.value(), time);
    require_equal(row.timestampN.value(), timestamp);
  }

  return 0;
}
