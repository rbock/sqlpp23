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

namespace test {
// Mock table and column definitions
struct TabFoo : public sqlpp::table_t<TabFoo, sqlpp::char_sequence<'f', 'o', 'o'>> {
  struct ColInt : sqlpp::column_t<TabFoo, sqlpp::integer_t, sqlpp::char_sequence<'c', 'o', 'l', '_', 'i', 'n', 't'>> {};
  struct ColIntNottnull : sqlpp::column_t<TabFoo, sqlpp::integer_t, sqlpp::char_sequence<'c', 'o', 'l', '_', 'i', 'n', 't', '_', 'n', 'o', 't', 't', 'n', 'u', 'l', 'l'>> {
    static constexpr auto has_not_null_constraint() -> bool { return true; }
  };
  struct ColText1 : sqlpp::column_t<TabFoo, sqlpp::text_t, sqlpp::char_sequence<'c', 'o', 'l', '_', 't', 'e', 'x', 't', '1'>> {
    static constexpr auto has_not_null_constraint() -> bool { return true; }
  };
  struct ColText2 : sqlpp::column_t<TabFoo, sqlpp::text_t, sqlpp::char_sequence<'c', 'o', 'l', '_', 't', 'e', 'x', 't', '2'>> {
    static constexpr auto has_not_null_constraint() -> bool { return true; }
  };
  struct ColText3 : sqlpp::column_t<TabFoo, sqlpp::text_t, sqlpp::char_sequence<'c', 'o', 'l', '_', 't', 'e', 'x', 't', '3'>> {
    static constexpr auto has_not_null_constraint() -> bool { return true; }
  };
  struct ColTextNullable : sqlpp::column_t<TabFoo, sqlpp::text_t, sqlpp::char_sequence<'c', 'o', 'l', '_', 't', 'e', 'x', 't', '_', 'n', 'u', 'l', 'l', 'a', 'b', 'l', 'e'>> {};

  struct ColBigintNottnull : sqlpp::column_t<TabFoo, sqlpp::bigint_t, sqlpp::char_sequence<'c', 'o', 'l', '_', 'b', 'i', 'g', 'i', 'n', 't', '_', 'n', 'o', 't', 't', 'n', 'u', 'l', 'l'>> {
    static constexpr auto has_not_null_constraint() -> bool { return true; }
  };
  struct ColBigintNullable : sqlpp::column_t<TabFoo, sqlpp::bigint_t, sqlpp::char_sequence<'c', 'o', 'l', '_', 'b', 'i', 'g', 'i', 'n', 't', '_', 'n', 'u', 'l', 'l', 'a', 'b', 'l', 'e'>> {};
  
  struct ColBool : sqlpp::column_t<TabFoo, sqlpp::boolean_t, sqlpp::char_sequence<'c', 'o', 'l', '_', 'b', 'o', 'o', 'l'>> {
    static constexpr auto has_not_null_constraint() -> bool { return true; }
  };
  struct ColFloat : sqlpp::column_t<TabFoo, sqlpp::floating_point_t, sqlpp::char_sequence<'c', 'o', 'l', '_', 'f', 'l', 'o', 'a', 't'>> {
    static constexpr auto has_not_null_constraint() -> bool { return true; }
  };
};

constexpr auto tab_foo = TabFoo{};
constexpr auto col_int = TabFoo::ColInt{};
constexpr auto col_int_nottnull = TabFoo::ColIntNottnull{};
constexpr auto col_text1 = TabFoo::ColText1{};
constexpr auto col_text2 = TabFoo::ColText2{};
constexpr auto col_text3 = TabFoo::ColText3{};
constexpr auto col_text_nullable = TabFoo::ColTextNullable{};
constexpr auto col_bigint_nottnull = TabFoo::ColBigintNottnull{};
constexpr auto col_bigint_nullable = TabFoo::ColBigintNullable{};
constexpr auto col_bool = TabFoo::ColBool{};
constexpr auto col_float = TabFoo::ColFloat{};

// Helper conditions for static tests
const auto c1 = col_int == 1; // Using potentially nullable col_int for conditions
const auto c2 = col_int == 2; // to ensure CASE logic handles it if needed.

} // namespace test

int main(int, char*[]) {
  using test::c1; // Bring conditions into scope
  using test::c2;
  using test::col_bigint_nullable;
  using test::col_bigint_nottnull;
  using test::col_bool;
  using test::col_float;
  using test::col_int;
  using test::col_int_nottnull;
  using test::col_text1;
  using test::col_text2;
  using test::col_text3;
  using test::col_text_nullable;
  // Keep existing test variables if they don't conflict
  const auto cond = sqlpp::value(true);
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

  // --- New tests for refactored fluent CASE expression ---
  using namespace sqlpp; // For brevity in static_asserts

  // **1. Basic Serialization:**
  SQLPP_COMPARE(::sqlpp::case_when(col_int == 1).then(col_text1).else_(col_text2),
                "CASE WHEN (foo.col_int = 1) THEN foo.col_text1 ELSE foo.col_text2 END");
  SQLPP_COMPARE(::sqlpp::case_when(col_int == 1).then(col_text1).when(col_int == 2).then(col_text2).else_(col_text3),
                "CASE WHEN (foo.col_int = 1) THEN foo.col_text1 WHEN (foo.col_int = 2) THEN foo.col_text2 ELSE foo.col_text3 END");
  SQLPP_COMPARE(::sqlpp::case_when(col_bool).then(sqlpp::value(1)).else_(sqlpp::value(0)),
                "CASE foo.col_bool THEN 1 ELSE 0 END");
  SQLPP_COMPARE(::sqlpp::case_when(col_int == 1).then(std::nullopt).else_(col_text1),
                "CASE WHEN (foo.col_int = 1) THEN NULL ELSE foo.col_text1 END");
  SQLPP_COMPARE(::sqlpp::case_when(col_int == 1).then(col_text1).else_(std::nullopt),
                "CASE WHEN (foo.col_int = 1) THEN foo.col_text1 ELSE NULL END");

  // **2. Type and Nullability Deduction (Static Tests):**

  // Scenario 1: All non-null, same type.
  {
    const auto expr_s1 = ::sqlpp::case_when(c1).then(col_text1).else_(col_text2);
    static_assert(std::is_same_v<data_type_of_t<decltype(expr_s1)>, std::string_view>, "Type deduction S1 failed");
    static_assert(can_be_null_v<decltype(expr_s1)> == false, "Nullability deduction S1 failed");
  }

  // Scenario 2: One THEN is nullable.
  {
    const auto expr_s2 = ::sqlpp::case_when(c1).then(col_text_nullable).else_(col_text1);
    static_assert(std::is_same_v<data_type_of_t<decltype(expr_s2)>, std::string_view>, "Type deduction S2 failed");
    static_assert(can_be_null_v<decltype(expr_s2)> == true, "Nullability deduction S2 failed");
  }
  { // First then is not null, second then is nullable
    const auto expr_s2_multi = ::sqlpp::case_when(c1).then(col_text1).when(c2).then(col_text_nullable).else_(col_text2);
    static_assert(std::is_same_v<data_type_of_t<decltype(expr_s2_multi)>, std::string_view>, "Type deduction S2 multi failed");
    static_assert(can_be_null_v<decltype(expr_s2_multi)> == true, "Nullability deduction S2 multi failed");
  }

  // Scenario 3: ELSE is nullable.
  {
    const auto expr_s3 = ::sqlpp::case_when(c1).then(col_text1).else_(col_text_nullable);
    static_assert(std::is_same_v<data_type_of_t<decltype(expr_s3)>, std::string_view>, "Type deduction S3 failed");
    static_assert(can_be_null_v<decltype(expr_s3)> == true, "Nullability deduction S3 failed");
  }

  // Scenario 4: First THEN establishes type (e.g., col_int_nottnull), subsequent THEN is different but comparable type (e.g., sqlpp::value(100LL) which is BIGINT).
  {
    const auto expr_s4 = ::sqlpp::case_when(c1).then(col_int_nottnull).when(c2).then(sqlpp::value(100LL)).else_(sqlpp::value(0));
    static_assert(std::is_same_v<data_type_of_t<decltype(expr_s4)>, std::int32_t>, "Type deduction S4 failed");
    static_assert(can_be_null_v<decltype(expr_s4)> == false, "Nullability deduction S4 failed");
  }

  // Scenario 5: First THEN is col_int_nottnull, ELSE is col_bigint_nullable.
  {
    const auto expr_s5 = ::sqlpp::case_when(c1).then(col_int_nottnull).else_(col_bigint_nullable);
    static_assert(std::is_same_v<data_type_of_t<decltype(expr_s5)>, std::int32_t>, "Type deduction S5 failed");
    static_assert(can_be_null_v<decltype(expr_s5)> == true, "Nullability deduction S5 failed");
  }

  // Scenario 6: First THEN is std::nullopt, subsequent is not_null TEXT
  // This test assumes that the ResultType of the CASE expression can be determined by a later
  // THEN or ELSE clause if the first THEN clause resolved to std::nullopt.
  // The current implementation of derive_case_data_type_and_nullability uses data_type_of_t<ResultType>,
  // where ResultType is set by the first .then(). If the first .then() is std::nullopt, ResultType becomes
  // std::nullopt_t, and data_type_of_t<std::nullopt_t> is void. This test might fail
  // unless case_builder_t or derive_case_data_type_and_nullability is enhanced to handle this.
  {
    const auto expr_s6 = ::sqlpp::case_when(c1).then(std::nullopt).else_(col_text1);
    static_assert(std::is_same_v<data_type_of_t<decltype(expr_s6)>, std::string_view>, "Type deduction S6 failed - see comment in test source");
    static_assert(can_be_null_v<decltype(expr_s6)> == true, "Nullability deduction S6 failed");
  }
  
  // Scenario 7: First THEN is not_null BIGINT, ELSE is not_null INT
   {
    const auto expr_s7 = ::sqlpp::case_when(c1).then(col_bigint_nottnull).else_(col_int_nottnull);
    static_assert(std::is_same_v<data_type_of_t<decltype(expr_s7)>, std::int64_t>, "Type deduction S7 failed");
    static_assert(can_be_null_v<decltype(expr_s7)> == false, "Nullability deduction S7 failed");
  }

  // **3. Type Comparability (Static Tests - Valid Cases):**
  { // int and bigint
    const auto expr_tc1 = ::sqlpp::case_when(c1).then(col_int_nottnull).else_(col_bigint_nottnull);
    static_assert(std::is_same_v<data_type_of_t<decltype(expr_tc1)>, std::int32_t>, "Type deduction TC1 failed");
    static_assert(can_be_null_v<decltype(expr_tc1)> == false, "Nullability deduction TC1 failed");
  }
  { // float and int (float is ResultType)
    const auto expr_tc2 = ::sqlpp::case_when(c1).then(col_float).else_(col_int_nottnull);
    static_assert(std::is_same_v<data_type_of_t<decltype(expr_tc2)>, double>, "Type deduction TC2 failed"); // float in sqlpp maps to double
    static_assert(can_be_null_v<decltype(expr_tc2)> == false, "Nullability deduction TC2 failed");
  }
  { // Multiple whens with different comparable types
    const auto expr_tc3 = ::sqlpp::case_when(c1).then(col_int_nottnull)        // ResultType = int32_t
                           .when(c2).then(col_bigint_nottnull) // comparable with int32_t
                           .else_(sqlpp::value(0.0f));        // comparable with int32_t
    static_assert(std::is_same_v<data_type_of_t<decltype(expr_tc3)>, std::int32_t>, "Type deduction TC3 failed");
    static_assert(can_be_null_v<decltype(expr_tc3)> == false, "Nullability deduction TC3 failed");
  }
  { // First .then is nullable, sets overall nullability
    const auto expr_tc4 = ::sqlpp::case_when(c1).then(col_int)                // ResultType = int32_t, nullable
                           .when(c2).then(col_bigint_nottnull) // comparable
                           .else_(sqlpp::value(0));           // comparable
    static_assert(std::is_same_v<data_type_of_t<decltype(expr_tc4)>, std::int32_t>, "Type deduction TC4 failed");
    static_assert(can_be_null_v<decltype(expr_tc4)> == true, "Nullability deduction TC4 failed");
  }

  return 0;
}
