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

#include <sqlpp26/tests/core/all.h>

namespace {
template <typename... Expressions>
concept can_call_exists_with =
    requires(Expressions... expressions) { sqlpp::exists(expressions...); };

template <typename... Expressions>
concept cannot_call_exists_with =
    not(can_call_exists_with<Expressions...>);
}  // namespace

int main() {
  const auto bar = test::tab_bar{};

  static_assert(cannot_call_exists_with<decltype(7)>);
  static_assert(cannot_call_exists_with<decltype(bar)>);
  static_assert(cannot_call_exists_with<decltype(bar.id)>);

  // Prepare-consistency is not required, e.g. missing tables can be provided by
  // the enclosing statement.
  {
    auto incomplete = sqlpp::select(bar.id);
    using Incomplete = decltype(incomplete);
    expect_basic_consistency_succeeds<Incomplete>();
    expect_prepare_consistency_fails<
        Incomplete,
        "at least one selected column requires a table which is otherwise not "
        "known in the statement">();

    static_assert(can_call_exists_with<Incomplete>);
    exists(incomplete);
  }

  // Basic consistency is required for a statement to be considered for `exists`
  // Calling exists is allowed but will throw a compile time exception.
  {
    constexpr auto inconsistent = sqlpp::select(bar.id).having(bar.int_n > 7);
    using Inconsistent = decltype(inconsistent);
    expect_basic_consistency_fails<
        Inconsistent,
        "having expression not built out of aggregate expressions">();

    static_assert(can_call_exists_with<Inconsistent>);
    // TODO: Need to test the thrown exception in a compile-failure test
  }

  // Multi-column selects can be used for `exists`
  {
    auto multi_incomplete = sqlpp::select(bar.id, bar.text_n);
    using MultiIncomplete = decltype(multi_incomplete);
    expect_basic_consistency_succeeds<MultiIncomplete>();
    expect_prepare_consistency_fails<
        MultiIncomplete,
        "at least one selected column requires a table which is otherwise not "
        "known in the statement">();

    static_assert(can_call_exists_with<MultiIncomplete>);
    exists(multi_incomplete);
  }
}
