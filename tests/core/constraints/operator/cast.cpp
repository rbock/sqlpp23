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
template <typename... Expressions>
concept can_call_cast_as_with =
    requires(Expressions... expressions) { sqlpp::cast(expressions...); };

template <typename... Expressions>
concept cannot_call_cast_as_with =
    not(can_call_cast_as_with<Expressions...>);
}  // namespace

int main() {
  const auto bar = test::tab_bar{};

  // Can call basics
  static_assert(can_call_cast_as_with<decltype(7), decltype(sqlpp::as<int>())>);
  static_assert(can_call_cast_as_with<decltype(bar.id), decltype(sqlpp::as<int>())>);

  // Can call with interesting type combinations.
  static_assert(can_call_cast_as_with<decltype(bar.id), decltype(sqlpp::as<std::string_view>())>);

  // Cannot all with just one argument
  static_assert(cannot_call_cast_as_with<decltype(7)>);
  static_assert(cannot_call_cast_as_with<decltype(bar)>);
  static_assert(cannot_call_cast_as_with<decltype(bar.id)>);

  // Cannot call cast on things without a data type
  static_assert(cannot_call_cast_as_with<decltype(bar), decltype(sqlpp::as<int>())>);

  // Cannot call cast directly with select
  static_assert(cannot_call_cast_as_with<decltype(select(bar.id).from(bar)), decltype(sqlpp::as<int>())>);
}
