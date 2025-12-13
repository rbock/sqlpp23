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

#include <sqlpp23/tests/core/all.h>

template <typename T, typename DataType>
using is_data_type = std::is_same<sqlpp::data_type_of_t<T>, DataType>;

template <typename Value>
void test_value(Value v) {
  using DataType = sqlpp::data_type_of_t<Value>;
  using OptDataType = std::optional<DataType>;

  auto v_not_null = sqlpp::value(v);
  auto v_maybe_null = sqlpp::value(std::optional{v});

  static_assert(is_data_type<decltype(v_not_null), DataType>::value, "");
  static_assert(is_data_type<decltype(v_maybe_null), OptDataType>::value, "");

  static_assert(not sqlpp::can_be_null<decltype(v_not_null)>::value, "");
  static_assert(sqlpp::can_be_null<decltype(v_maybe_null)>::value, "");

  static_assert(sqlpp::has_enabled_as<decltype(v_not_null)>::value, "");
  static_assert(sqlpp::has_enabled_as<decltype(v_maybe_null)>::value, "");

  static_assert(sqlpp::has_enabled_comparison<decltype(v_not_null)>::value, "");
  static_assert(sqlpp::has_enabled_comparison<decltype(v_maybe_null)>::value,
                "");

  static_assert(std::is_same<sqlpp::nodes_of_t<decltype(v_not_null)>,
                             sqlpp::detail::type_vector<Value>>::value,
                "");
  static_assert(
      std::is_same<sqlpp::nodes_of_t<decltype(v_maybe_null)>,
                   sqlpp::detail::type_vector<std::optional<Value>>>::value,
      "");
}

void test_value_of_select_as() {
  const auto foo = test::TabFoo{};

  auto s = select(foo.id).from(foo).where(foo.id == 17);
  using S = decltype(s);
  auto v = value(s);
  static_assert(std::is_same<sqlpp::nodes_of_t<decltype(v)>,
                             sqlpp::detail::type_vector<S>>::value,
                "");
}

int main() {
  // boolean
  test_value(bool{true});

  // integral
  test_value(int8_t{7});
  test_value(int16_t{7});
  test_value(int32_t{7});
  test_value(int64_t{7});

  // unsigned integral
  test_value(uint8_t{7});
  test_value(uint16_t{7});
  test_value(uint32_t{7});
  test_value(uint64_t{7});

  // floating point
  test_value(7.7f);
  test_value(double{7.7});

  // text
  test_value('7');
  test_value("seven");
  test_value(std::string("seven"));
  test_value(std::string_view("seven"));

  // blob
  test_value(std::vector<uint8_t>{});

  // date
  test_value(std::chrono::sys_days{});

  // timestamp
  test_value(::sqlpp::chrono::sys_microseconds{});
  using minute_point =
      std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>;
  test_value(minute_point{});

  // time
  test_value(std::chrono::microseconds{});

  // Test value(select(...))
  test_value_of_select_as();
}
