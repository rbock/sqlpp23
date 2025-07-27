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

#include <sqlpp23/tests/core/all.h>

namespace {
template <typename... Expressions>
concept can_call_coalesce_with =
    requires(Expressions... expressions) { sqlpp::coalesce(expressions...); };
}  // namespace

int main() {
  const auto maybe = true;
  const auto foo = test::TabFoo{};

  // coalesce(<non arguments>) cannot be called.
  static_assert(not can_call_coalesce_with<>);

  // coalesce(mixed arguments) cannot be called.
  static_assert(not can_call_coalesce_with<int, float>);
  static_assert(not can_call_coalesce_with<std::chrono::sys_days,
                                           sqlpp::chrono::sys_microseconds>);

  // coalesce(<one or more same-type arguments>) is OK
  static_assert(can_call_coalesce_with<decltype("a")>);
  static_assert(can_call_coalesce_with<decltype("a"), decltype("b")>);
  static_assert(can_call_coalesce_with<std::string_view, decltype(foo.textNnD)>);
  static_assert(
      can_call_coalesce_with<decltype(dynamic(maybe, foo.textNnD)), std::string>);

  // coalesce(<plain NULL>) does not work as the type is unclear.
  // However, you can, if you really want to specify the type, of course.
  static_assert(not can_call_coalesce_with<decltype(std::nullopt)>);
  static_assert(can_call_coalesce_with<decltype(std::optional<int>{})>);
}
