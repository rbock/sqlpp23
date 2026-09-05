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
// Returns true if `assign(declcal<Lhs>, declval<Rhs>())` is a valid function
// call.
template <typename Lhs, typename Rhs>
concept can_call_assign_with = requires(Lhs lhs, Rhs rhs) {
  lhs = rhs;
  sqlpp::assign(lhs, rhs);
};
}  // namespace

int main() {
  const auto maybe = true;
  const auto foo = test::tab_foo{};
  const auto bar = test::tab_bar{};
  const auto date_time = test::tab_date_time{};

  // OK
  bar.int_n = 7;
  bar.int_n = sqlpp::default_value;
  static_assert(can_call_assign_with<decltype(bar.int_n), decltype(7)>);

  // Cannot assign wrong value type or other stuff like tables.
  static_assert(can_call_assign_with<decltype(bar.bool_nn), decltype(true)>);
  static_assert(
      not can_call_assign_with<decltype(bar.bool_nn), decltype("cheesecake")>);
  static_assert(not can_call_assign_with<decltype(bar.bool_nn), decltype(bar)>);
  static_assert(
      not can_call_assign_with<decltype(bar.bool_nn),
                               decltype(sqlpp::dynamic(maybe, true))>);

  date_time.timestamp_n = ::sqlpp::chrono::sys_microseconds{};
  // Can assign different std::chrono::sys_time types to timestamp
  static_assert(
      can_call_assign_with<decltype(date_time.timestamp_n),
                           decltype(::sqlpp::chrono::sys_microseconds{})>);

  static_assert(can_call_assign_with<
                decltype(date_time.timestamp_n),
                decltype(std::chrono::sys_time<std::chrono::seconds>{})>);

  // Must not mix date and date_time in assignments, see e.g.
  // https://github.com/rbock/sqlpp23/issues/26
  static_assert(
      not can_call_assign_with<decltype(date_time.date_n),
                               decltype(::sqlpp::chrono::sys_microseconds{})>);
  static_assert(not can_call_assign_with<decltype(date_time.date_n),
                                         decltype(date_time.timestamp_n)>);

  static_assert(not can_call_assign_with<decltype(date_time.timestamp_n),
                                         decltype(std::chrono::sys_days{})>);
  static_assert(not can_call_assign_with<decltype(date_time.timestamp_n),
                                         decltype(date_time.date_n)>);

  // std::chrono::sys_time<Period> can be assigned to timestamp columns if
  // `Period{1} < std::chrono::days{1}`.
  using week_point =
      std::chrono::time_point<std::chrono::system_clock, std::chrono::weeks>;
  using hour_point =
      std::chrono::time_point<std::chrono::system_clock, std::chrono::hours>;
  using minute_point =
      std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>;

  static_assert(not can_call_assign_with<decltype(date_time.timestamp_n),
                                         decltype(week_point{})>);

  static_assert(can_call_assign_with<decltype(date_time.timestamp_n),
                                     decltype(hour_point{})>);

  static_assert(can_call_assign_with<decltype(date_time.timestamp_n),
                                     decltype(minute_point{})>);

  static_assert(can_call_assign_with<decltype(date_time.timestamp_n),
                                     decltype(date_time.timestamp_n)>);

  // Non-nullable without default cannot be assigned null / default
  static_assert(not can_call_assign_with<decltype(bar.bool_nn),
                                         decltype(std::optional{true})>);
  static_assert(
      not can_call_assign_with<decltype(bar.bool_nn), decltype(std::nullopt)>);
  static_assert(not can_call_assign_with<decltype(bar.bool_nn),
                                         decltype(sqlpp::default_value)>);

  // Non-nullable with default cannot be assigned null, but default
  static_assert(
      can_call_assign_with<decltype(foo.text_nn_d), decltype("cake")>);
  static_assert(not can_call_assign_with<decltype(foo.text_nn_d),
                                         decltype(std::optional{"cake"})>);
  static_assert(not can_call_assign_with<decltype(foo.text_nn_d),
                                         decltype(std::nullopt)>);
  static_assert(can_call_assign_with<decltype(foo.text_nn_d),
                                     decltype(sqlpp::default_value)>);

  // Const column cannot be assigned anything
  const auto const_col = foo.int_c_n;
  static_assert(not can_call_assign_with<decltype(const_col), decltype(7)>);
  static_assert(
      not can_call_assign_with<decltype(const_col), decltype(std::optional{7})>);
  static_assert(
      not can_call_assign_with<decltype(const_col), decltype(std::nullopt)>);
  static_assert(not can_call_assign_with<decltype(const_col),
                                         decltype(sqlpp::default_value)>);
}
