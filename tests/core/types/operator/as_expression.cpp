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

SQLPP_CREATE_NAME_TAG(cheese);

template <typename T, typename DataType>
using is_select_column_data_type =
    std::is_same<sqlpp::select_column_data_type_of_t<T>, DataType>;

template <typename Value>
void test_as_expression(Value v) {
  auto v_not_null = sqlpp::value(v).as(cheese);
  auto v_maybe_null = sqlpp::value(std::optional{v}).as(cheese);
  auto v_dynamic_not_null = dynamic(true, sqlpp::value(v).as(cheese));
  auto v_dynamic_maybe_null =
      dynamic(true, sqlpp::value(std::optional{v}).as(cheese));

  static_assert(not sqlpp::has_data_type<decltype(v_not_null)>::value, "");
  static_assert(not sqlpp::has_data_type<decltype(v_maybe_null)>::value, "");
  static_assert(not sqlpp::has_data_type<decltype(v_dynamic_not_null)>::value,
                "");
  static_assert(
      not sqlpp::has_data_type<decltype(v_dynamic_maybe_null)>::value, "");

  static_assert(sqlpp::has_name_tag<decltype(v_not_null)>::value, "");
  static_assert(sqlpp::has_name_tag<decltype(v_maybe_null)>::value, "");
  static_assert(not sqlpp::has_name_tag<decltype(v_dynamic_not_null)>::value,
                "");
  static_assert(not sqlpp::has_name_tag<decltype(v_dynamic_maybe_null)>::value,
                "");

  // AS expressions have do not enable the `as` member function.
  static_assert(not sqlpp::has_enabled_as<decltype(v_not_null)>::value, "");

  // AS expressions do not enable comparison member functions.
  static_assert(not sqlpp::has_enabled_comparison<decltype(v_not_null)>::value,
                "");

  // AS expressions have their arguments as nodes.
  using L = typename std::decay<decltype(sqlpp::value(v))>::type;
  static_assert(std::is_same<sqlpp::nodes_of_t<decltype(v_not_null)>,
                             sqlpp::detail::type_vector<L>>::value,
                "");
}

int main() {
  // boolean
  test_as_expression(bool{true});

  // integral
  test_as_expression(int8_t{7});
  test_as_expression(int16_t{7});
  test_as_expression(int32_t{7});
  test_as_expression(int64_t{7});

  // unsigned integral
  test_as_expression(uint8_t{7});
  test_as_expression(uint16_t{7});
  test_as_expression(uint32_t{7});
  test_as_expression(uint64_t{7});

  // floating point
  test_as_expression(float{7.7});
  test_as_expression(double{7.7});

  // text
  test_as_expression('7');
  test_as_expression("seven");
  test_as_expression(std::string("seven"));
  test_as_expression(std::string_view("seven"));

  // blob
  test_as_expression(std::vector<uint8_t>{});

  // date
  test_as_expression(std::chrono::sys_days{});

  // timestamp
  test_as_expression(::sqlpp::chrono::sys_microseconds{});
  using minute_point =
      std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>;
  test_as_expression(minute_point{});

  // time
  test_as_expression(std::chrono::microseconds{});
}
