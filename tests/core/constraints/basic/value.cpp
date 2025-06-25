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

SQLPP_CREATE_NAME_TAG(something);

template <typename... Expressions>
concept can_call_value_with =
    requires(Expressions... expressions) { sqlpp::value(expressions...); };
template <typename... Expressions>
concept cannot_call_value_with =
    not(can_call_value_with<Expressions...>);
}  // namespace

int main() {
  const auto foo = test::TabFoo{};
  auto s = select(foo.id).from(foo);

  // OK
  static_assert(can_call_value_with<int>);
  static_assert(can_call_value_with<std::string>);
  static_assert(can_call_value_with<decltype(s)>);

  static_assert(can_call_value_with<std::optional<int>>);
  static_assert(can_call_value_with<std::optional<std::string>>);

  // Not OK
  static_assert(cannot_call_value_with<decltype(foo)>);
  static_assert(cannot_call_value_with<decltype(foo.id)>);
  static_assert(cannot_call_value_with<decltype(foo.id == 7)>);
  static_assert(cannot_call_value_with<decltype(foo.id + 7)>);
  static_assert(cannot_call_value_with<decltype((foo.id + 7).as(something))>);
  static_assert(cannot_call_value_with<decltype(sqlpp::value(7))>);
  static_assert(cannot_call_value_with<decltype(std::optional{s})>);
}
