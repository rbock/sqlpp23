/*
 * Copyright (c) 2026, Vesselin Atanasov
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

// A sample program demonstrating reading/writing of enum values.
//
// For details on the actual pattern see /docs/recipes/enum.md

#include <sqlpp23/tests/postgresql/all.h>

// The enum definitions must be included before the database model
#include <enum_animal.h>
#include <enum_shape.h>
#include <db_model.h>

#include <ranges>

namespace sql = ::sqlpp::postgresql;

void test_enum_rw(sql::connection& db) {
  auto te = test::TabEnums{};
  auto val_write = std::vector<std::pair<animal, shape>>{
      // For the write/read check below to work, these values should be ordered
      // by the first member ("animal") in ascending order and then by the
      // second member ("cage") again in ascending order
      {animal::bird, shape::triangle},
      {animal::cat, shape::circle},
      {animal::dog, shape::square},
      {static_cast<animal>(10), shape::circle}};
  // Write the enums vector to the database
  delete_from(te);
  for (const auto v : val_write) {
    db(insert_into(te).set(te.animal = v.first, te.cage = v.second));
  }
  // Read the enums vector from the database
  auto val_read =
      db(select(all_of(te)).from(te).order_by(te.animal.asc(), te.cage.asc())) |
      std::views::transform(
          [](const auto& row) { return std::pair{*row.animal, *row.cage}; }) |
      std::ranges::to<std::vector>();
  // Check if the written and read vectors are equal
  if (val_write != val_read) {
    throw std::runtime_error{"Written and read data differs"};
  }
}

int main() {
  auto db = sql::make_test_connection();
  db("DROP TYPE IF EXISTS shape CASCADE");
  db("CREATE TYPE shape AS ENUM ('circle', 'square', 'triangle')");
  test::createTabEnums(db);
  test_enum_rw(db);
  return 0;
}
