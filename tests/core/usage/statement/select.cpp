/*
 * Copyright (c) 2025, Roland Bock
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

#include <ranges>
#include <tuple>

#include <sqlpp23/tests/core/all.h>

SQLPP_CREATE_NAME_TAG(cheese);

namespace {
struct my_data {
  int64_t id;
  std::string textNnD;
};
auto row_to_data = [](const auto& row) -> my_data {
  return {row.id, std::string{row.textNnD}};
};
}  // namespace

int main(int, char*[]) {
  try {
    auto db = sqlpp::mock_db::make_test_connection();

    const auto foo = test::TabFoo{};

    // select value
    for (const auto& row : db(select(sqlpp::value(23).as(cheese)))) {
      std::ignore = row.cheese;
    }

    // select single column
    for (const auto& row : db(select(foo.id).from(foo))) {
      std::ignore = row.id;
    }

    // select two columns
    for (const auto& row : db(select(foo.id, foo.textNnD).from(foo))) {
      std::ignore = row.id;
      std::ignore = row.textNnD;
    }

    // select all columns
    for (const auto& row : db(select(all_of(foo)).from(foo))) {
      std::ignore = row.id;
      std::ignore = row.textNnD;
      std::ignore = row.intN;
    }

    // use std::views to transform result
    {
      auto result = db(select(foo.id, foo.textNnD).from(foo));
      const auto vec = result | std::ranges::views::transform(row_to_data) |
                       std::ranges::to<std::vector>();
    }

    // As of v21 libc++ does not support std::ranges::views::enumerate
    // For details see: https://libcxx.llvm.org/Status/Cxx23.html
#ifndef _LIBCPP_VERSION
    for ([[maybe_unused]] const auto& [index, row] :
         db(select(foo.id).from(foo)) | std::ranges::views::enumerate) {
      // do something with index and row
    }
#endif
    // We can use zip with iota to create enumerate-like functionality
    for ([[maybe_unused]] const auto& [index, row] :
         std::ranges::views::zip(std::views::iota(0uz), db(select(foo.id).from(foo)))) {
      // do something with index and row
    }

  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
