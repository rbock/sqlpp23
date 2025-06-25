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

SQLPP_CREATE_NAME_TAG(r_not_null);
SQLPP_CREATE_NAME_TAG(r_maybe_null);

template <typename T, typename DataType>
using is_select_column_data_type =
    std::is_same<sqlpp::select_column_data_type_of_t<T>, DataType>;

template <typename Value>
void test_dynamic(Value v) {
  using DataType = sqlpp::data_type_of_t<Value>;
  using OptDataType = std::optional<DataType>;

  auto v_not_null = dynamic(true, sqlpp::value(v));
  auto v_maybe_null = dynamic(true, sqlpp::value(std::optional{v}));
  auto v_alias = sqlpp::value(v).as(r_not_null);
  auto v_maybe_alias = sqlpp::value(std::optional{v}).as(r_maybe_null);
  auto v_not_null_alias = dynamic(true, v_alias);
  auto v_maybe_null_alias = dynamic(true, v_maybe_alias);

  static_assert(not sqlpp::has_data_type<decltype(v_not_null)>::value, "");
  static_assert(not sqlpp::has_data_type<decltype(v_maybe_null)>::value, "");
  static_assert(not sqlpp::has_data_type<decltype(v_not_null_alias)>::value,
                "");
  static_assert(not sqlpp::has_data_type<decltype(v_maybe_null_alias)>::value,
                "");

  static_assert(not sqlpp::has_name_tag<decltype(v_not_null)>::value, "");
  static_assert(not sqlpp::has_name_tag<decltype(v_maybe_null)>::value, "");
  static_assert(not sqlpp::has_name_tag<decltype(v_not_null_alias)>::value, "");
  static_assert(not sqlpp::has_name_tag<decltype(v_maybe_null_alias)>::value,
                "");

  static_assert(
      is_select_column_data_type<decltype(v_not_null), OptDataType>::value,
      "");
  static_assert(
      is_select_column_data_type<decltype(v_maybe_null), OptDataType>::value,
      "");
  static_assert(is_select_column_data_type<decltype(v_not_null_alias),
                                            OptDataType>::value,
                "");
  static_assert(is_select_column_data_type<decltype(v_maybe_null_alias),
                                            OptDataType>::value,
                "");

  static_assert(not sqlpp::select_column_has_name<decltype(v_not_null)>::value,
                "");
  static_assert(
      not sqlpp::select_column_has_name<decltype(v_maybe_null)>::value, "");
  static_assert(
      sqlpp::select_column_has_name<decltype(v_not_null_alias)>::value, "");
  static_assert(
      sqlpp::select_column_has_name<decltype(v_maybe_null_alias)>::value, "");

  static_assert(
      std::is_same<sqlpp::nodes_of_t<decltype(v_not_null)>,
                   sqlpp::detail::type_vector<
                       sqlpp::remove_dynamic_t<decltype(v_not_null)>>>::value,
      "");
  static_assert(
      std::is_same<sqlpp::nodes_of_t<decltype(v_maybe_null)>,
                   sqlpp::detail::type_vector<
                       sqlpp::remove_dynamic_t<decltype(v_maybe_null)>>>::value,
      "");
  static_assert(
      std::is_same<sqlpp::nodes_of_t<decltype(v_not_null_alias)>,
                   sqlpp::detail::type_vector<decltype(v_alias)>>::value,
      "");
  static_assert(
      std::is_same<sqlpp::nodes_of_t<decltype(v_maybe_null_alias)>,
                   sqlpp::detail::type_vector<decltype(v_maybe_alias)>>::value,
      "");
}

int main() {
  // boolean
  test_dynamic(bool{true});

  // integral
  test_dynamic(int8_t{7});
  test_dynamic(int16_t{7});
  test_dynamic(int32_t{7});
  test_dynamic(int64_t{7});

  // unsigned integral
  test_dynamic(uint8_t{7});
  test_dynamic(uint16_t{7});
  test_dynamic(uint32_t{7});
  test_dynamic(uint64_t{7});

  // floating point
  test_dynamic(float{7.7});
  test_dynamic(double{7.7});

  // text
  test_dynamic('7');
  test_dynamic("seven");
  test_dynamic(std::string("seven"));
  test_dynamic(std::string_view("seven"));

  // blob
  test_dynamic(std::vector<uint8_t>{});

  // date
  test_dynamic(std::chrono::sys_days{});

  // timestamp
  test_dynamic(::sqlpp::chrono::sys_microseconds{});
  using minute_point =
      std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>;
  test_dynamic(minute_point{});

  // time
  test_dynamic(std::chrono::microseconds{});
}
