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
#include <sqlpp23/mock_db/database/connection.h>
#include <sqlpp23/tests/core/tables.h>

namespace {
template <typename T, typename V>
using is_same_type = std::is_same<sqlpp::data_type_of_t<T>, V>;
}

template <typename Value>
void test_avg(Value v) {
  auto v_not_null = sqlpp::value(v);
  auto v_maybe_null = sqlpp::value(std::make_optional(v));

  using ValueType = sqlpp::data_type_of_t<Value>;
  using OptValueType = sqlpp::data_type_of_t<std::optional<Value>>;

  // if any arg is nullable, then the result is nullable
  static_assert(is_same_type<decltype(coalesce(v_not_null)), ValueType>::value);
  static_assert(is_same_type<decltype(coalesce(v_not_null, v_not_null)), ValueType>::value);
  static_assert(is_same_type<decltype(coalesce(v_maybe_null)), OptValueType>::value);
  static_assert(is_same_type<decltype(coalesce(v_maybe_null, v_not_null)), OptValueType>::value);
  static_assert(is_same_type<decltype(coalesce(v_not_null, v_maybe_null)), OptValueType>::value);

  // coalesce enables the `as` member function.
  static_assert(sqlpp::has_enabled_as<decltype(coalesce(v_not_null))>::value);

  // coalesce has no name
  static_assert(not sqlpp::has_name_tag<decltype(coalesce(v_not_null))>::value);

  // coalesce enables comparison member functions.
  static_assert(sqlpp::has_enabled_comparison<decltype(coalesce(v_not_null))>::value,
                "");

  // coalesce has its argument as nodes
  using A = typename std::decay<decltype(v_not_null)>::type;
  using B = typename std::decay<decltype(v_maybe_null)>::type;
  static_assert(
      std::is_same<
          sqlpp::nodes_of_t<decltype(coalesce(v_not_null, v_maybe_null))>,
          sqlpp::detail::type_vector<A, B>>::value,
      "");
}

int main() {
  // boolean
  test_avg(bool{true});

  // integral
  test_avg(int64_t{7});

  // unsigned integral
  test_avg(uint64_t{7});

  // string point
  test_avg("seven");
}
