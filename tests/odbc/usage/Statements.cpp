/*
 * Copyright (c) 2026, Leander Schulten
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

#include <algorithm>
#include <cstdlib>
#include <ranges>
#include <vector>

#include <sqlpp23/tests/odbc/all.h>

// Exercises the execution paths of the connector: direct and prepared
// select/insert/update/delete, all data types, NULL values, block fetch
// across rowset boundaries, the SQLGetData streaming fallback, and
// transactions.
//
// Without a configured data source this only verifies that everything
// compiles: set SQLPP_ODBC_CONNECTION_STRING to run it against a real
// database (the DDL in tables.h may need adjustment for your database).
namespace {

#define expect(condition)                                                     \
  if (not(condition)) {                                                       \
    throw std::runtime_error{std::format("{}:{}: check failed: {}", __FILE__, \
                                         __LINE__, #condition)};              \
  }

template <typename Db>
size_t count_rows(Db& db) {
  const auto foo = test::TabFoo{};
  size_t count = 0;
  for (const auto& row : db(select(foo.id).from(foo))) {
    (void)row;
    ++count;
  }
  return count;
}

void test_data_type_round_trips(sqlpp::odbc::connection& db) {
  const auto foo = test::TabFoo{};
  test::createTabFoo(db);

  auto blob_value = std::vector<uint8_t>(100 * 1000);
  for (size_t index = 0; index < blob_value.size(); ++index) {
    blob_value[index] = static_cast<uint8_t>(index % 251);
  }

  // Insert with direct execution (values become literals), read back.
  db(insert_into(foo).set(foo.textNnD = "it's a text", foo.intN = -42,
                          foo.doubleN = 1.25,
                          foo.uIntN = uint64_t{1234567890123456789},
                          foo.boolN = true, foo.blobN = blob_value));

  size_t row_count = 0;
  for (const auto& row :
       db(select(all_of(foo)).from(foo).where(foo.textNnD == "it's a text"))) {
    ++row_count;
    expect(row.textNnD == "it's a text");
    expect(row.intN == -42);
    expect(row.doubleN == 1.25);
    expect(row.uIntN == uint64_t{1234567890123456789});
    expect(row.boolN == true);
    expect(row.blobN.has_value());
    expect(std::ranges::equal(*row.blobN, blob_value));
  }
  expect(row_count == 1);

  // Insert the same values as parameters of a prepared statement.
  auto prepared_insert = db.prepare(insert_into(foo).set(
      foo.textNnD = parameter(foo.textNnD), foo.intN = parameter(foo.intN),
      foo.doubleN = parameter(foo.doubleN), foo.uIntN = parameter(foo.uIntN),
      foo.boolN = parameter(foo.boolN), foo.blobN = parameter(foo.blobN)));
  prepared_insert.parameters.textNnD = "prepared";
  prepared_insert.parameters.intN = -43;
  prepared_insert.parameters.doubleN = 2.5;
  prepared_insert.parameters.uIntN = uint64_t{987654321};
  prepared_insert.parameters.boolN = false;
  prepared_insert.parameters.blobN = blob_value;
  db(prepared_insert);

  auto prepared_select =
      db.prepare(select(all_of(foo))
                     .from(foo)
                     .where(foo.textNnD == parameter(foo.textNnD)));
  prepared_select.parameters.textNnD = "prepared";
  row_count = 0;
  for (const auto& row : db(prepared_select)) {
    ++row_count;
    expect(row.intN == -43);
    expect(row.doubleN == 2.5);
    expect(row.uIntN == uint64_t{987654321});
    expect(row.boolN == false);
    expect(row.blobN.has_value());
    expect(std::ranges::equal(*row.blobN, blob_value));
  }
  expect(row_count == 1);

  // NULL values, direct and as parameters.
  db(update(foo)
         .set(foo.intN = std::nullopt, foo.doubleN = std::nullopt,
              foo.uIntN = std::nullopt, foo.boolN = std::nullopt,
              foo.blobN = std::nullopt)
         .where(foo.textNnD == "it's a text"));
  prepared_insert.parameters.textNnD = "nulls";
  prepared_insert.parameters.intN = std::nullopt;
  prepared_insert.parameters.doubleN = std::nullopt;
  prepared_insert.parameters.uIntN = std::nullopt;
  prepared_insert.parameters.boolN = std::nullopt;
  prepared_insert.parameters.blobN = std::nullopt;
  db(prepared_insert);

  for (const auto& row :
       db(select(all_of(foo))
              .from(foo)
              .where(foo.textNnD == "it's a text" or foo.textNnD == "nulls"))) {
    expect(row.intN == std::nullopt);
    expect(row.doubleN == std::nullopt);
    expect(row.uIntN == std::nullopt);
    expect(row.boolN == std::nullopt);
    expect(row.blobN == std::nullopt);
  }

  // Update and delete report affected rows.
  expect(db(update(foo).set(foo.intN = 1).where(foo.textNnD == "nulls"))
             .affected_rows == 1);
  expect(db(delete_from(foo).where(foo.textNnD == "nulls")).affected_rows == 1);
}

void test_date_time_round_trips(sqlpp::odbc::connection& db) {
  using namespace std::chrono_literals;
  const auto tab = test::TabDateTime{};
  test::createTabDateTime(db);

  const auto date_value =
      static_cast<std::chrono::sys_days>(std::chrono::February / 8 / 2025);
  // The connector preserves microseconds, but some drivers round timestamp
  // parameters (e.g. the SQLite driver stores milliseconds), so use a
  // millisecond-precision value to keep this test portable.
  const auto timestamp_value = date_value + 10h + 9min + 8s + 123ms;
  // The ODBC time type has no sub-second precision.
  const auto time_value = 10h + 9min + 8s;

  // Insert as literals ({d ...}, {ts ...}, {t ...} escapes).
  db(insert_into(tab).set(tab.dateN = date_value,
                          tab.timestampN = timestamp_value,
                          tab.timeN = std::chrono::microseconds{time_value}));

  // Insert the same values as parameters (SQL_C_TYPE_* structs).
  auto prepared_insert = db.prepare(
      insert_into(tab).set(tab.dateN = parameter(tab.dateN),
                           tab.timestampN = parameter(tab.timestampN),
                           tab.timeN = parameter(tab.timeN)));
  prepared_insert.parameters.dateN = date_value;
  prepared_insert.parameters.timestampN = timestamp_value;
  prepared_insert.parameters.timeN = std::chrono::microseconds{time_value};
  db(prepared_insert);

  size_t row_count = 0;
  for (const auto& row :
       db(select(tab.dateN, tab.timestampN, tab.timeN).from(tab))) {
    ++row_count;
    expect(row.dateN == date_value);
    expect(row.timestampN == timestamp_value);
    expect(row.timeN == time_value);
  }
  expect(row_count == 2);
}

// Fetching more rows than fit into one rowset must cross rowset boundaries
// correctly (db is configured with a small row_array_size).
void test_multiple_rowsets(sqlpp::odbc::connection& db) {
  const auto foo = test::TabFoo{};
  test::createTabFoo(db);

  auto prepared_insert = db.prepare(insert_into(foo).set(
      foo.textNnD = "row", foo.intN = parameter(foo.intN)));
  constexpr int64_t row_count = 10;
  for (int64_t i = 0; i < row_count; ++i) {
    prepared_insert.parameters.intN = i;
    db(prepared_insert);
  }

  // varchar(255) is bindable, so this uses block fetch, crossing rowset
  // boundaries and reading text from the bound column arrays.
  int64_t expected = 0;
  for (const auto& row :
       db(select(foo.textNnD, foo.intN).from(foo).order_by(foo.intN.asc()))) {
    expect(row.textNnD == "row");
    expect(row.intN == expected);
    ++expected;
  }
  expect(expected == row_count);
}

// With a tiny max_bound_column_size, text columns cannot be bound and the
// result falls back to streaming values via SQLGetData (db is configured
// accordingly). Values larger than the initial streaming buffer exercise
// the buffer growth loop.
void test_streamed_text(sqlpp::odbc::connection& db) {
  const auto foo = test::TabFoo{};
  test::createTabFoo(db);

  const auto long_text = std::string(5000, 'x') + "end";
  auto prepared_insert = db.prepare(
      insert_into(foo).set(foo.textNnD = parameter(foo.textNnD), foo.intN = 7));
  prepared_insert.parameters.textNnD = long_text;
  db(prepared_insert);
  db(insert_into(foo).set(foo.textNnD = "", foo.intN = std::nullopt));

  size_t row_count = 0;
  for (const auto& row :
       db(select(foo.textNnD, foo.intN).from(foo).order_by(foo.id.asc()))) {
    if (row_count == 0) {
      expect(row.textNnD == long_text);
      expect(row.intN == 7);
    } else {
      expect(row.textNnD == "");
      expect(row.intN == std::nullopt);
    }
    ++row_count;
  }
  expect(row_count == 2);
}

void test_transactions(sqlpp::odbc::connection& db) {
  const auto foo = test::TabFoo{};
  test::createTabFoo(db);

  {
    auto tx = start_transaction(db);
    db(insert_into(foo).set(foo.textNnD = "committed"));
    tx.commit();
  }
  expect(count_rows(db) == 1);
  expect(not db.is_transaction_active());

  {
    auto tx = start_transaction(db);
    db(insert_into(foo).set(foo.textNnD = "rolled back"));
    expect(db.is_transaction_active());
    tx.rollback();
  }
  expect(count_rows(db) == 1);
  expect(not db.is_transaction_active());
}
}  // namespace

int main() {
  if (std::getenv("SQLPP_ODBC_CONNECTION_STRING") == nullptr) {
    std::println(
        "ODBC: skipping statement execution, SQLPP_ODBC_CONNECTION_STRING is "
        "not set");
    return 0;
  }

  try {
    {
      auto db = sqlpp::odbc::make_test_connection();
      expect(db.is_connected());
      test_data_type_round_trips(db);
      test_date_time_round_trips(db);
      test_transactions(db);
    }
    {
      // Small rowsets to test fetching across rowset boundaries.
      auto config = sqlpp::odbc::make_test_config();
      config->row_array_size = 3;
      sqlpp::odbc::connection db;
      db.connect_using(config);
      test_multiple_rowsets(db);
    }
    {
      // Tiny column-buffer limit to force the SQLGetData streaming fallback.
      auto config = sqlpp::odbc::make_test_config();
      config->max_bound_column_size = 8;
      sqlpp::odbc::connection db;
      db.connect_using(config);
      test_streamed_text(db);
    }
  } catch (const std::exception& e) {
    std::println(stderr, "Exception: {}", e.what());
    return -1;
  }

  std::println("ODBC: all statement tests passed");
  return 0;
}
