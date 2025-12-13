/*
 * Copyright (c) 2019, Roland Bock, Jaroslav Bisikirski
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
#include <functional>
#include <random>

#include <sqlpp23/tests/sqlite3/all.h>

namespace sql = sqlpp::sqlite3;
const auto tab = test::TabFoo{};

/*
 * max default blob/text is 1,000,000,000
 * But 999,999,993 is the biggest one SQLITE will accept
 *  without throwing SQLITE_TOOBIG
 * But it takes too long (over 5s) to generate and check
 */
constexpr size_t blob_size = 1000 * 1000ul;
constexpr size_t blob_small_size = 999;

void verify_blob(sql::connection& db,
                 const std::vector<uint8_t>& expected,
                 uint64_t id) {
  auto result = db(select(tab.blobN).from(tab).where(tab.id == id));
  const auto& result_row = result.front();
  if (!result_row.blobN)
    throw std::runtime_error("blob blobN is unpexpectedly NULL for id " +
                             std::to_string(id));
  const auto received = *result_row.blobN;

  if (expected.size() != received.size()) {
    std::cerr << "Size mismatch" << std::endl;

    throw std::runtime_error("Size mismatch " +
                             std::to_string(expected.size()) +
                             " != " + std::to_string(received.size()));
  }
  std::cerr << "Verifying content" << std::endl;
  for (size_t i = 0; i < expected.size(); i++) {
    if (expected[i] != received[i]) {
      std::cerr << "expected[" << i << "] " << static_cast<int>(expected[i])
                << " != received " << static_cast<int>(received[i])
                << std::endl;
      throw std::runtime_error("Content mismatch");
    }
  }
}

int Blob(int, char*[]) {
  auto db = sql::make_test_connection();
  test::createTabFoo(db);
  std::cerr << "Generating blobN " << blob_size << std::endl;
  std::vector<uint8_t> blobN(blob_size);
  std::uniform_int_distribution<unsigned short> distribution(0, 255);
  std::mt19937 engine;
  auto generator = std::bind(distribution, engine);
  std::generate_n(blobN.begin(), blob_size, generator);

  std::vector<uint8_t> smallBlob(blob_small_size);
  std::generate_n(blobN.begin(), blob_small_size, generator);

  // If we use the bigger blob it will trigger SQLITE_TOOBIG for the query
  auto result = db(insert_into(tab).set(tab.blobN = smallBlob));

  auto prepared_insert =
      db.prepare(insert_into(tab).set(tab.blobN = parameter(tab.blobN)));
  prepared_insert.parameters.blobN = blobN;
  const auto prep_result = db(prepared_insert);
  prepared_insert.parameters.blobN = std::nullopt;
  const auto null_result = db(prepared_insert);

  verify_blob(db, smallBlob, result.last_insert_id);
  verify_blob(db, blobN, prep_result.last_insert_id);
  {
    auto result2 = db(select(tab.blobN).from(tab).where(tab.id == null_result.last_insert_id));
    const auto& result_row = result2.front();
    std::cerr << "Null blob is_null:\t" << std::boolalpha
              << (result_row.blobN == std::nullopt) << std::endl;
  }
  return 0;
}
