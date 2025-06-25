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
// Returns true if `assign(declcal<Lhs>, declval<Rhs>())` is a valid function
// call.
template <typename Lhs, typename Rhs, typename = void>
struct can_call_assign_with : public std::false_type {};

template <typename Lhs, typename Rhs>
struct can_call_assign_with<
    Lhs,
    Rhs,
    std::void_t<decltype(assign(std::declval<Lhs>(), std::declval<Rhs>()))>>
    : public std::true_type {};
}  // namespace

int main() {
  const auto maybe = true;
  const auto foo = test::TabFoo{};
  const auto bar = test::TabBar{};
  const auto date_time = test::TabDateTime{};

  // OK
  bar.intN = 7;
  bar.intN = sqlpp::default_value;
  static_assert(can_call_assign_with<decltype(bar.intN), decltype(7)>::value, "");

  // Cannot assign wrong value type or other stuff like tables.
  static_assert(
      can_call_assign_with<decltype(bar.boolNn), decltype(true)>::value, "");
  static_assert(not can_call_assign_with<decltype(bar.boolNn),
                                         decltype("cheesecake")>::value,
                "");
  static_assert(
      not can_call_assign_with<decltype(bar.boolNn), decltype(bar)>::value, "");
  static_assert(
      not can_call_assign_with<decltype(bar.boolNn),
                               decltype(sqlpp::dynamic(maybe, true))>::value,
      "");

  date_time.timestampN = ::sqlpp::chrono::sys_microseconds{};
  // Can assign different std::chrono::sys_time types to timestamp
  static_assert(
      can_call_assign_with<decltype(date_time.timestampN),
                               decltype(::sqlpp::chrono::sys_microseconds{})>::value);

  static_assert(
      can_call_assign_with<decltype(date_time.timestampN),
                               decltype(std::chrono::sys_time<std::chrono::seconds>{})>::value);


  // Must not mix date and date_time in assignments, see e.g.
  // https://github.com/rbock/sqlpp23/issues/26
  static_assert(not can_call_assign_with<
                decltype(date_time.dateN),
                decltype(::sqlpp::chrono::sys_microseconds{})>::value);
  static_assert(not can_call_assign_with<
                decltype(date_time.dateN),
                decltype(date_time.timestampN)>::value);

  static_assert(
      not can_call_assign_with<decltype(date_time.timestampN),
                               decltype(std::chrono::sys_days{})>::value);
  static_assert(
      not can_call_assign_with<decltype(date_time.timestampN),
                               decltype(date_time.dateN)>::value);

  // std::chrono::sys_time<Period> can be assigned to timestamp columns if
  // `Period{1} < std::chrono::days{1}`.
  using week_point =
      std::chrono::time_point<std::chrono::system_clock, std::chrono::weeks>;
  using hour_point =
      std::chrono::time_point<std::chrono::system_clock, std::chrono::hours>;
  using minute_point =
      std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>;

  static_assert(not can_call_assign_with<decltype(date_time.timestampN),
                                         decltype(week_point{})>::value);

  static_assert(can_call_assign_with<decltype(date_time.timestampN),
                                         decltype(hour_point{})>::value);

  static_assert(can_call_assign_with<decltype(date_time.timestampN),
                                         decltype(minute_point{})>::value);

  static_assert(can_call_assign_with<decltype(date_time.timestampN),
                                         decltype(date_time.timestampN)>::value);

  // Non-nullable without default cannot be assigned null / default
  static_assert(
      not can_call_assign_with<decltype(bar.boolNn),
                               decltype(std::optional{true})>::value,
      "");
  static_assert(not can_call_assign_with<decltype(bar.boolNn),
                                         decltype(std::nullopt)>::value,
                "");
  static_assert(not can_call_assign_with<decltype(bar.boolNn),
                                         decltype(sqlpp::default_value)>::value,
                "");

  // Non-nullable with default cannot be assigned null, but default
  static_assert(
      can_call_assign_with<decltype(foo.textNnD), decltype("cake")>::value, "");
  static_assert(
      not can_call_assign_with<decltype(foo.textNnD),
                               decltype(std::optional{"cake"})>::value,
      "");
  static_assert(not can_call_assign_with<decltype(foo.textNnD),
                                         decltype(std::nullopt)>::value,
                "");
  static_assert(can_call_assign_with<decltype(foo.textNnD),
                                     decltype(sqlpp::default_value)>::value,
                "");

  // Const column cannot be assigned anything
  const auto const_col = foo.intCN;
  static_assert(
      not can_call_assign_with<decltype(const_col), decltype(7)>::value, "");
  static_assert(
      not can_call_assign_with<decltype(const_col),
                               decltype(std::optional{7})>::value,
      "");
  static_assert(not can_call_assign_with<decltype(const_col),
                                         decltype(std::nullopt)>::value,
                "");
  static_assert(not can_call_assign_with<decltype(const_col),
                                         decltype(sqlpp::default_value)>::value,
                "");
}
