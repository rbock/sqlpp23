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

#include <sqlpp26/tests/core/all.h>

namespace {
template <typename Lhs>
concept can_call_as_on = requires(const Lhs& lhs) { lhs.template as<"something">(); };

template <typename Lhs>
concept can_call_as_with = requires(const Lhs& lhs) { sqlpp::as<"something">(lhs); };

}  // namespace

int main() {
  const auto maybe = true;
  constexpr auto bar = test::tab_bar{};

  // OK
  static_assert(can_call_as_on<decltype(sqlpp::value(maybe))>);
  static_assert(can_call_as_on<decltype(bar.id)>);
  static_assert(can_call_as_on<decltype(bar)>);
  static_assert(can_call_as_on<decltype(sqlpp::cte<"sample">().as(
                    select(bar.id).from(bar)))>);
  static_assert(can_call_as_on<decltype(select(bar.id).from(bar))>);

  // OK, functions can be named
  static_assert(can_call_as_on<decltype(max(bar.bool_nn))>);

  // Can call as on incomplete select statement, but will receive a compile time
  // exception
  auto incomplete_select = sqlpp::select(bar.id);
  static_assert(can_call_as_on<decltype(incomplete_select)>);
  // TODO: Need to test the compile-time exception in a compile failure test

  // Cannot name non-select statements
  static_assert(not can_call_as_on<decltype(update(bar).set(bar.id = 7))>);

  // dynamic cannot be named can be named
  static_assert(not can_call_as_on<decltype(dynamic(maybe, bar.bool_nn))>);

  // Renamed things cannot be renamed again.
  static_assert(not can_call_as_on<decltype(bar.id.as<"something">())>);

  // Built-in types don't have an .as member function
  static_assert(not can_call_as_on<bool>);

  // However, it would be possible to call the stand-alone as() function
  static_assert(can_call_as_with<bool>);
}
