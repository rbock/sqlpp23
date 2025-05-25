/*
 * Copyright (c) 2024, Roland Bock
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
#include <sqlpp23/tests/core/serialize_helpers.h>
#include <sqlpp23/tests/core/tables.h>
#include "sqlpp23/core/type_traits/data_type.h"

int main(int, char*[]) {
  constexpr auto foo = test::TabFoo{};
  constexpr auto bar = test::TabBar{};

  // Keep existing test variables if they don't conflict
  const auto cond = sqlpp::value(true);
  const auto cond2 = sqlpp::value(false);
  const auto val = sqlpp::value(11);
  const auto expr = sqlpp::value(17) + 4;

  // Case operands use parentheses where required.
  SQLPP_COMPARE(case_when(cond).then(val).else_(val),
                "CASE WHEN 1 THEN 11 ELSE 11 END");
  SQLPP_COMPARE(case_when(cond).then(val).else_(expr),
                "CASE WHEN 1 THEN 11 ELSE (17 + 4) END");
  SQLPP_COMPARE(case_when(cond).then(expr).else_(val),
                "CASE WHEN 1 THEN (17 + 4) ELSE 11 END");
  SQLPP_COMPARE(case_when(cond).then(expr).else_(expr),
                "CASE WHEN 1 THEN (17 + 4) ELSE (17 + 4) END");
  SQLPP_COMPARE(case_when(false or cond).then(val).else_(val),
                "CASE WHEN (0 OR 1) THEN 11 ELSE 11 END");
  SQLPP_COMPARE(case_when(false or cond).then(val).else_(expr),
                "CASE WHEN (0 OR 1) THEN 11 ELSE (17 + 4) END");
  SQLPP_COMPARE(case_when(false or cond).then(expr).else_(val),
                "CASE WHEN (0 OR 1) THEN (17 + 4) ELSE 11 END");
  SQLPP_COMPARE(case_when(false or cond).then(expr).else_(expr),
                "CASE WHEN (0 OR 1) THEN (17 + 4) ELSE (17 + 4) END");

  // **1. Basic Serialization:**
  SQLPP_COMPARE(::sqlpp::case_when(foo.intN == 1).then(foo.textNnD).else_("something else"),
                "CASE WHEN (foo.col_int = 1) THEN foo.foo.textNnD ELSE foo.col_text2 END");
  SQLPP_COMPARE(sqlpp::case_when(foo.intN == 1).then(foo.textNnD).when(foo.intN == 2).then("2").else_("something else"),
                "CASE WHEN (foo.col_int = 1) THEN foo.foo.textNnD WHEN (foo.col_int = 2) THEN foo.col_text2 ELSE foo.col_text3 END");
  SQLPP_COMPARE(::sqlpp::case_when(foo.intN == 1).then(sqlpp::value(1)).else_(sqlpp::value(0)),
                "CASE foo.col_bool THEN 1 ELSE 0 END");
  SQLPP_COMPARE(::sqlpp::case_when(foo.intN == 1).then(std::optional<std::string>{}).else_(foo.textNnD),
                "CASE WHEN (foo.col_int = 1) THEN NULL ELSE foo.foo.textNnD END");
  SQLPP_COMPARE(::sqlpp::case_when(foo.intN == 1).then(foo.textNnD).else_(std::nullopt),
                "CASE WHEN (foo.col_int = 1) THEN foo.foo.textNnD ELSE NULL END");

  // **2. Type and Nullability Deduction (Static Tests):**

  // Scenario 1: All non-null, same type.
  {
    auto expr_s1 = ::sqlpp::case_when(cond).then(foo.textNnD).else_("cheese");
    static_assert(std::is_same_v<sqlpp::data_type_of_t<decltype(expr_s1)>, sqlpp::text>, "Type deduction S1 failed");
    static_assert(not sqlpp::can_be_null<decltype(expr_s1)>::value, "Nullability deduction S1 failed");
  }

  // Scenario 2: One THEN is nullable.
  {
    auto expr_s2 = ::sqlpp::case_when(cond).then(bar.textN).else_(foo.textNnD);
    static_assert(std::is_same_v<sqlpp::data_type_of_t<decltype(expr_s2)>, std::optional<sqlpp::text>>, "Type deduction S2 failed");
    static_assert(sqlpp::can_be_null<decltype(expr_s2)>::value == true, "Nullability deduction S2 failed");
  }
  { // First then is not null, second then is nullable
    auto expr_s2_multi = ::sqlpp::case_when(cond).then(foo.textNnD).when(cond2).then(bar.textN).else_("something");
    static_assert(std::is_same_v<sqlpp::data_type_of_t<decltype(expr_s2_multi)>, std::optional<sqlpp::text>>, "Type deduction S2 multi failed");
    static_assert(sqlpp::can_be_null<decltype(expr_s2_multi)>::value, "Nullability deduction S2 multi failed");
  }

  // Scenario 3: ELSE is nullable.
  {
    auto expr_s3 = ::sqlpp::case_when(cond).then(foo.textNnD).else_(bar.textN);
    static_assert(std::is_same_v<sqlpp::data_type_of_t<decltype(expr_s3)>, std::optional<sqlpp::text>>, "Type deduction S3 failed");
    static_assert(sqlpp::can_be_null<decltype(expr_s3)>::value, "Nullability deduction S3 failed");
  }

  // Scenario 4: First THEN establishes type (e.g., bar.id), subsequent THEN is different but comparable type (e.g., sqlpp::value(100LL) which is BIGINT).
  {
    auto expr_s4 = ::sqlpp::case_when(cond).then(bar.id).when(cond2).then(sqlpp::value(100l)).else_(sqlpp::value(0));
    static_assert(std::is_same_v<sqlpp::data_type_of_t<decltype(expr_s4)>, sqlpp::integral>, "Type deduction S4 failed");
    static_assert(not sqlpp::can_be_null<decltype(expr_s4)>::value, "Nullability deduction S4 failed");
  }

  // Scenario 5: First THEN is bar.id, ELSE is foo.intN.
  {
    auto expr_s5 = ::sqlpp::case_when(cond).then(bar.id).else_(foo.intN);
    static_assert(std::is_same_v<sqlpp::data_type_of_t<decltype(expr_s5)>, std::optional<sqlpp::integral>>, "Type deduction S5 failed");
    static_assert(sqlpp::can_be_null<decltype(expr_s5)>::value, "Nullability deduction S5 failed");
  }

  // Scenario 6: First THEN is std::nullopt, subsequent is not_null TEXT
  // This test assumes that the ResultType of the CASE expression can be determined by a later
  // THEN or ELSE clause if the first THEN clause resolved to std::nullopt.
  // The current implementation of derive_case_data_type_and_nullability uses data_type_of_t<ResultType>,
  // where ResultType is set by the first .then(). If the first .then() is std::nullopt, ResultType becomes
  // std::nullopt_t, and data_type_of_t<std::nullopt_t> is void. This test might fail
  // unless case_builder_t or derive_case_data_type_and_nullability is enhanced to handle this.
  {
    auto expr_s6 = ::sqlpp::case_when(cond).then(std::optional<std::string>{}).else_(foo.textNnD);
    static_assert(std::is_same_v<sqlpp::data_type_of_t<decltype(expr_s6)>, std::optional<sqlpp::text>>, "Type deduction S6 failed - see comment in test source");
    static_assert(sqlpp::can_be_null<decltype(expr_s6)>::value, "Nullability deduction S6 failed");
  }
  
  // Scenario 7: First THEN is not_null BIGINT, ELSE is not_null INT
   {
    auto expr_s7 = ::sqlpp::case_when(cond).then(foo.id).else_(bar.id);
    static_assert(std::is_same_v<sqlpp::data_type_of_t<decltype(expr_s7)>, sqlpp::integral>, "Type deduction S7 failed");
    static_assert(not sqlpp::can_be_null<decltype(expr_s7)>::value, "Nullability deduction S7 failed");
  }

  // **3. Type Comparability (Static Tests - Valid Cases):**
  { // int and bigint
    auto expr_tc1 = ::sqlpp::case_when(cond).then(bar.id).else_(foo.id);
    static_assert(std::is_same_v<sqlpp::data_type_of_t<decltype(expr_tc1)>, sqlpp::integral>, "Type deduction TC1 failed");
    static_assert(not sqlpp::can_be_null<decltype(expr_tc1)>::value, "Nullability deduction TC1 failed");
  }
  { // float and int (float is ResultType)
    auto expr_tc2 = ::sqlpp::case_when(cond).then(foo.doubleN).else_(bar.id);
    static_assert(std::is_same_v<sqlpp::data_type_of_t<decltype(expr_tc2)>, std::optional<sqlpp::floating_point>>, "Type deduction TC2 failed"); // float in sqlpp maps to double
    static_assert(sqlpp::can_be_null<decltype(expr_tc2)>::value, "Nullability deduction TC2 failed");
  }
  { // Multiple whens with different comparable types
    auto expr_tc3 =
        ::sqlpp::case_when(cond).then(bar.id).when(cond2).then(foo.id).else_(
            sqlpp::value(0.0f));
    static_assert(std::is_same_v<sqlpp::data_type_of_t<decltype(expr_tc3)>, sqlpp::integral>, "Type deduction TC3 failed");
    static_assert(not sqlpp::can_be_null<decltype(expr_tc3)>::value, "Nullability deduction TC3 failed");
  }
  { // First .then is nullable, sets overall nullability
    auto expr_tc4 = ::sqlpp::case_when(cond).then(foo.intN)                // ResultType = int32_t, nullable
                           .when(cond2).then(foo.id) // comparable
                           .else_(sqlpp::value(0));           // comparable
    static_assert(std::is_same_v<sqlpp::data_type_of_t<decltype(expr_tc4)>, std::optional<sqlpp::integral>>, "Type deduction TC4 failed");
    static_assert(sqlpp::can_be_null<decltype(expr_tc4)>::value, "Nullability deduction TC4 failed");
  }

  return 0;
}
