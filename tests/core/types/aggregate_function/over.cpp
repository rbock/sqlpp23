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
#include <sqlpp23/mock_db/database/connection.h>
#include <sqlpp23/tests/core/tables.h>

namespace {
template <typename T, typename V>
using is_same_type = std::is_same<sqlpp::data_type_of_t<T>, V>;
}

template <typename Value>
void test_aggregate_functions(Value v) {
  auto v_not_null = sqlpp::value(v);
  auto v_maybe_null = sqlpp::value(std::optional{v});

  using OptDataType = sqlpp::data_type_of_t<std::optional<Value>>;

  // Aggregate of non-nullable
  static_assert(
      is_same_type<decltype(count(v_not_null).over()), sqlpp::integral>::value,
      "");
  static_assert(
      is_same_type<decltype(max(v_not_null).over()), OptDataType>::value, "");
  static_assert(
      is_same_type<decltype(min(v_not_null).over()), OptDataType>::value, "");

  // Aggregate of nullable
  static_assert(
      is_same_type<decltype(count(v_not_null).over()), sqlpp::integral>::value,
      "");
  static_assert(
      is_same_type<decltype(max(v_maybe_null).over()), OptDataType>::value,
      "");
  static_assert(
      is_same_type<decltype(min(v_maybe_null).over()), OptDataType>::value,
      "");

  // Aggregate functions enable the `as` member function.
  static_assert(
      sqlpp::has_enabled_as<decltype(count(v_not_null).over())>::value, "");
  static_assert(sqlpp::has_enabled_as<decltype(max(v_not_null).over())>::value,
                "");
  static_assert(sqlpp::has_enabled_as<decltype(min(v_not_null).over())>::value,
                "");

  // Aggregate functions have a name
  static_assert(
      not sqlpp::has_name_tag<decltype(count(v_not_null).over())>::value, "");
  static_assert(
      not sqlpp::has_name_tag<decltype(max(v_not_null).over())>::value, "");
  static_assert(
      not sqlpp::has_name_tag<decltype(min(v_not_null).over())>::value, "");

  // Aggregate functions enable comparison member functions.
  static_assert(
      sqlpp::has_enabled_comparison<decltype(count(v_not_null).over())>::value,
      "");
  static_assert(
      sqlpp::has_enabled_comparison<decltype(max(v_not_null).over())>::value,
      "");
  static_assert(
      sqlpp::has_enabled_comparison<decltype(min(v_not_null).over())>::value,
      "");

  // Aggregate functions enable OVER.
  static_assert(
      not sqlpp::has_enabled_over<decltype(count(v_not_null).over())>::value,
      "");
  static_assert(
      not sqlpp::has_enabled_over<decltype(max(v_not_null).over())>::value, "");
  static_assert(
      not sqlpp::has_enabled_over<decltype(min(v_not_null).over())>::value, "");

  // Aggregate functions have their arguments as nodes
  using L = typename std::decay<decltype(v_not_null)>::type;
  static_assert(
      std::is_same<sqlpp::nodes_of_t<decltype(count(v_not_null).over())>,
                   sqlpp::detail::type_vector<L>>::value,
      "");
  static_assert(
      std::is_same<sqlpp::nodes_of_t<decltype(max(v_not_null).over())>,
                   sqlpp::detail::type_vector<L>>::value,
      "");
  static_assert(
      std::is_same<sqlpp::nodes_of_t<decltype(min(v_not_null).over())>,
                   sqlpp::detail::type_vector<L>>::value,
      "");
}

template <typename Value>
void test_numeric_aggregate_functions(Value v) {
  auto v_not_null = sqlpp::value(v);
  auto v_maybe_null = sqlpp::value(std::optional{v});

  using DataType = typename std::conditional<std::is_same<Value, bool>::value,
                                              int, Value>::type;
  using OptDataType = sqlpp::data_type_of_t<std::optional<DataType>>;
  using OptFloat = sqlpp::data_type_of_t<std::optional<float>>;

  // Aggregate of non-nullable
  static_assert(
      is_same_type<decltype(sum(v_not_null).over()), OptDataType>::value, "");
  static_assert(is_same_type<decltype(avg(v_not_null).over()), OptFloat>::value,
                "");

  // Aggregate of nullable
  static_assert(
      is_same_type<decltype(sum(v_maybe_null).over()), OptDataType>::value,
      "");
  static_assert(
      is_same_type<decltype(avg(v_maybe_null).over()), OptFloat>::value, "");

  // Aggregate functions enable the `as` member function.
  static_assert(sqlpp::has_enabled_as<decltype(sum(v_not_null).over())>::value,
                "");
  static_assert(sqlpp::has_enabled_as<decltype(avg(v_not_null).over())>::value,
                "");

  // Aggregate functions have a name
  static_assert(
      not sqlpp::has_name_tag<decltype(sum(v_not_null).over())>::value, "");
  static_assert(
      not sqlpp::has_name_tag<decltype(avg(v_not_null).over())>::value, "");

  // Aggregate functions enable OVER.
  static_assert(
      not sqlpp::has_enabled_over<decltype(sum(v_not_null).over())>::value, "");
  static_assert(
      not sqlpp::has_enabled_over<decltype(avg(v_not_null).over())>::value, "");

  // Aggregate functions enable comparison member functions.
  static_assert(
      sqlpp::has_enabled_comparison<decltype(sum(v_not_null).over())>::value,
      "");
  static_assert(
      sqlpp::has_enabled_comparison<decltype(avg(v_not_null).over())>::value,
      "");

  // Aggregate functions have their arguments as nodes
  using L = typename std::decay<decltype(v_not_null)>::type;
  static_assert(
      std::is_same<sqlpp::nodes_of_t<decltype(sum(v_not_null).over())>,
                   sqlpp::detail::type_vector<L>>::value,
      "");
  static_assert(
      std::is_same<sqlpp::nodes_of_t<decltype(avg(v_not_null).over())>,
                   sqlpp::detail::type_vector<L>>::value,
      "");
}

int main() {
  // boolean
  test_aggregate_functions(bool{true});
  test_numeric_aggregate_functions(bool{true});

  // integral
  test_aggregate_functions(int8_t{7});
  test_aggregate_functions(int16_t{7});
  test_aggregate_functions(int32_t{7});
  test_aggregate_functions(int64_t{7});

  test_numeric_aggregate_functions(int8_t{7});
  test_numeric_aggregate_functions(int16_t{7});
  test_numeric_aggregate_functions(int32_t{7});
  test_numeric_aggregate_functions(int64_t{7});

  // unsigned integral
  test_aggregate_functions(uint8_t{7});
  test_aggregate_functions(uint16_t{7});
  test_aggregate_functions(uint32_t{7});
  test_aggregate_functions(uint64_t{7});

  test_numeric_aggregate_functions(uint8_t{7});
  test_numeric_aggregate_functions(uint16_t{7});
  test_numeric_aggregate_functions(uint32_t{7});
  test_numeric_aggregate_functions(uint64_t{7});

  // floating point
  test_aggregate_functions(float{7.7});
  test_aggregate_functions(double{7.7});

  test_numeric_aggregate_functions(float{7.7});
  test_numeric_aggregate_functions(double{7.7});

  // text
  test_aggregate_functions('7');
  test_aggregate_functions("seven");
  test_aggregate_functions(std::string("seven"));
  test_aggregate_functions(std::string_view("seven"));

  // blob
  test_aggregate_functions(std::vector<uint8_t>{});

  // date
  test_aggregate_functions(std::chrono::sys_days{});

  // timestamp
  test_aggregate_functions(::sqlpp::chrono::sys_microseconds{});
  using minute_point =
      std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>;
  test_aggregate_functions(minute_point{});

  // time
  test_aggregate_functions(std::chrono::microseconds{});
}
