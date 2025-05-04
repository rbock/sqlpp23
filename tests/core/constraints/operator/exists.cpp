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

#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/tests/core/tables.h>

namespace {
template <typename... Expressions>
concept can_call_exists_with =
    requires(Expressions... expressions) { sqlpp::exists(expressions...); };

template <typename... Expressions>
concept cannot_call_exists_with =
    not(can_call_exists_with<Expressions...>);
}  // namespace

int main() {
  const auto bar = test::TabBar{};

  static_assert(cannot_call_exists_with<decltype(7)>);
  static_assert(cannot_call_exists_with<decltype(bar)>);
  static_assert(cannot_call_exists_with<decltype(bar.id)>);

  // Prepare-consistency is not required, e.g. missing tables can be provided by
  // the enclosing statement.
  {
    auto incomplete_select = sqlpp::select(bar.id);
    static_assert(
        std::is_same<decltype(check_basic_consistency(incomplete_select)),
                     sqlpp::consistent_t>::value);
    static_assert(
        std::is_same<
            decltype(check_prepare_consistency(incomplete_select)),
            sqlpp::assert_no_unknown_tables_in_selected_columns_t>::value);

    static_assert(can_call_exists_with<decltype(incomplete_select)>);
  }

  // Basic consistency is required for a statement to be considered for `exists`
  {
    auto inconsistent_select = sqlpp::select(bar.id).having(bar.intN > 7);
    static_assert(
        std::is_same<decltype(check_basic_consistency(inconsistent_select)),
                     sqlpp::assert_having_all_aggregates_t>::value);
    static_assert(cannot_call_exists_with<decltype(inconsistent_select)>);
  }

  // Multi-column selects can be used for `exists`
  {
    auto bad_select = sqlpp::select(bar.id, bar.textN);
    static_assert(
        std::is_same<decltype(check_basic_consistency(bad_select)),
                     sqlpp::consistent_t>::value);
    static_assert(not sqlpp::has_data_type<decltype(check_basic_consistency(
                      bad_select))>::value);
    static_assert(can_call_exists_with<decltype(bad_select)>);
  }
}
