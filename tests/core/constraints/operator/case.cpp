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

// Returns true if `case_when(declcal<Lhs>)` is a valid function call.
template <typename... Expressions>
concept can_call_case_when_with =
    requires(Expressions... expressions) { sqlpp::case_when(expressions...); };

template <typename Lhs, typename... Expressions>
concept can_call_when_with =
    requires(Lhs lhs, Expressions... expressions) { lhs.when(expressions...); };

template <typename Lhs, typename... Expressions>
concept can_call_then_with =
    requires(Lhs lhs, Expressions... expressions) { lhs.then(expressions...); };

template <typename Lhs, typename... Expressions>
concept can_call_else_with =
    requires(Lhs lhs, Expressions... expressions) {
      lhs.else_(expressions...);
    };

}  // namespace

int main() {
  const auto maybe = true;
  const auto foo = test::TabFoo{};
  const auto bar = test::TabBar{};
  const auto dt = test::TabDateTime{};

  // -----------------------
  // case_when()
  // -----------------------

  // OK
  static_assert(can_call_case_when_with<decltype(true)>);
  static_assert(
      can_call_case_when_with<decltype(std::make_optional(true))>);
  static_assert(can_call_case_when_with<decltype(foo.boolN)>);
  static_assert(can_call_case_when_with<decltype(bar.boolNn)>);
  static_assert(can_call_case_when_with<decltype(bar.boolNn == true)>);
  static_assert(can_call_case_when_with<decltype(count(foo.id) > 0)>);
  static_assert(can_call_case_when_with<decltype(std::nullopt)>);

  // Fail: Cannot call case_when with renamed boolean
  static_assert(
      not can_call_case_when_with<decltype(bar.boolNn.as(something))>);
  // Fail: Cannot call case_when with non-boolean expressions.
  static_assert(not can_call_case_when_with<decltype(bar.id)>);
  static_assert(not can_call_case_when_with<decltype(bar.boolNn = true)>);
  static_assert(not can_call_case_when_with<decltype(bar)>);

  // -----------------------
  // case_when.then()
  // -----------------------
  {
    auto cw = sqlpp::case_when(maybe);
    using CW = decltype(cw);

    // OK
    static_assert(can_call_then_with<CW, decltype(bar.id)>);
    static_assert(can_call_then_with<CW, decltype(bar.textN)>);
    static_assert(
        can_call_then_with<CW,
                           decltype(std::optional<int>(std::nullopt))>);
    // OK, can call `then` with naked `nullopt`, but will need a non-nullopt `then` or `else_` later.
    static_assert(can_call_then_with<CW, decltype(std::nullopt)>);

    // Fail: Anything that does not have a value.
    static_assert(
        not can_call_then_with<CW, decltype(bar.boolNn = true)>);
    static_assert(
        not can_call_then_with<CW, decltype(bar.boolNn.as(something))>);
    static_assert(not can_call_then_with<CW, decltype(bar)>);
  }

  // -----------------------
  // case_when.then(<nullable text>)
  // -----------------------
  {
    auto cw = sqlpp::case_when(maybe).then(bar.textN);
    using CW = decltype(cw);

    // OK
    static_assert(can_call_else_with<CW, decltype(bar.textN)>);
    static_assert(can_call_else_with<CW, decltype(foo.textNnD)>);
    static_assert(can_call_else_with<CW, decltype(std::optional<std::string>(
                                             std::nullopt))>);

    // OK: the value type of CASE is determined by the THEN expression.
    static_assert(can_call_else_with<CW, decltype(std::nullopt)>);

    // Fail: Anything that does not the right value type:
    static_assert(
        not can_call_else_with<CW, decltype(bar.boolNn)>);
    static_assert(
        not can_call_else_with<CW, decltype(bar.intN)>);

    // Fail: Anything that does not have a value.
    static_assert(
        not can_call_else_with<CW, decltype(bar.boolNn = true)>);
    static_assert(
        not can_call_else_with<CW, decltype(bar.boolNn.as(something))>);
    static_assert(not can_call_else_with<CW, decltype(bar)>);

    auto cw2 = sqlpp::case_when(maybe).then(bar.textN).when(maybe);
    using CW2 = decltype(cw2);

    // OK
    static_assert(can_call_then_with<CW2, decltype(bar.textN)>);
    static_assert(can_call_then_with<CW2, decltype(foo.textNnD)>);
    static_assert(can_call_then_with<CW2, decltype(std::optional<std::string>(
                                             std::nullopt))>);

    // OK: the value type of CASE is determined by the first THEN expression.
    static_assert(can_call_then_with<CW2, decltype(std::nullopt)>);

    // Fail: Anything that does not the right value type:
    static_assert(
        not can_call_then_with<CW2, decltype(bar.boolNn)>);
    static_assert(
        not can_call_then_with<CW2, decltype(bar.intN)>);

    // Fail: Anything that does not have a value.
    static_assert(
        not can_call_then_with<CW2, decltype(bar.boolNn = true)>);
    static_assert(
        not can_call_then_with<CW2, decltype(bar.boolNn.as(something))>);
    static_assert(not can_call_then_with<CW2, decltype(bar)>);
  }

  // -----------------------
  // case_when.then(integral).else_()
  // -----------------------
  {
    auto cw = sqlpp::case_when(maybe).then(bar.textN);
    using CW = decltype(cw);

    // OK
    static_assert(can_call_else_with<CW, decltype(bar.textN)>);
    static_assert(can_call_else_with<CW, decltype(foo.textNnD)>);
    static_assert(can_call_else_with<CW, decltype(std::optional<std::string>(
                                             std::nullopt))>);

    // OK: the value type of CASE is determined by the THEN expression.
    static_assert(can_call_else_with<CW, decltype(std::nullopt)>);

    // Fail: Anything that does not the right value type:
    static_assert(
        not can_call_else_with<CW, decltype(bar.boolNn)>);
    static_assert(
        not can_call_else_with<CW, decltype(bar.intN)>);

    // Fail: Anything that does not have a value.
    static_assert(
        not can_call_else_with<CW, decltype(bar.boolNn = true)>);
    static_assert(
        not can_call_else_with<CW, decltype(bar.boolNn.as(something))>);
    static_assert(not can_call_else_with<CW, decltype(bar)>);

    auto cw2 = cw.when(maybe);
    using CW2 = decltype(cw2);

    // OK
    static_assert(can_call_then_with<CW2, decltype(bar.textN)>);
    static_assert(can_call_then_with<CW2, decltype(foo.textNnD)>);
    static_assert(can_call_then_with<CW2, decltype(std::optional<std::string>(
                                             std::nullopt))>);

    // OK: the value type of CASE is determined by the first THEN expression.
    static_assert(can_call_then_with<CW2, decltype(std::nullopt)>);

    // Fail: Anything that does not the right value type:
    static_assert(
        not can_call_then_with<CW2, decltype(bar.boolNn)>);
    static_assert(
        not can_call_then_with<CW2, decltype(bar.intN)>);

    // Fail: Anything that does not have a value.
    static_assert(
        not can_call_then_with<CW2, decltype(bar.boolNn = true)>);
    static_assert(
        not can_call_then_with<CW2, decltype(bar.boolNn.as(something))>);
    static_assert(not can_call_then_with<CW2, decltype(bar)>);
  }

  // -----------------------
  // case_when.then(<non-nullable text>).else_()
  // -----------------------
  {
    auto cw = sqlpp::case_when(maybe).then(foo.textNnD);
    using CW = decltype(cw);

    // OK
    static_assert(can_call_else_with<CW, decltype(bar.textN)>);
    static_assert(can_call_else_with<CW, decltype(foo.textNnD)>);
    static_assert(can_call_else_with<CW, decltype(std::optional<std::string>(
                                             std::nullopt))>);

    // OK: the value type of CASE is determined by the THEN expression.
    static_assert(can_call_else_with<CW, decltype(std::nullopt)>);

    // Fail: Anything that does not have a value.
    static_assert(
        not can_call_else_with<CW, decltype(bar.boolNn = true)>);
    static_assert(
        not can_call_else_with<CW, decltype(bar.boolNn.as(something))>);
    static_assert(not can_call_else_with<CW, decltype(bar)>);

    auto cw2 = cw.when(maybe).then(bar.textN).when(maybe);
    using CW2 = decltype(cw2);

    // OK
    static_assert(can_call_then_with<CW2, decltype(bar.textN)>);
    static_assert(can_call_then_with<CW2, decltype(foo.textNnD)>);
    static_assert(can_call_then_with<CW2, decltype(std::optional<std::string>(
                                             std::nullopt))>);

    // OK: the value type of CASE is determined by the first THEN expression.
    static_assert(can_call_then_with<CW2, decltype(std::nullopt)>);

    // Fail: Anything that does not the right value type:
    static_assert(
        not can_call_then_with<CW2, decltype(bar.boolNn)>);
    static_assert(
        not can_call_then_with<CW2, decltype(bar.intN)>);

    // Fail: Anything that does not have a value.
    static_assert(
        not can_call_then_with<CW2, decltype(bar.boolNn = true)>);
    static_assert(
        not can_call_then_with<CW2, decltype(bar.boolNn.as(something))>);
    static_assert(not can_call_then_with<CW2, decltype(bar)>);
  }

  // -----------------------
  // case_when.then(nullopt)
  // -----------------------
  {
    auto cw = sqlpp::case_when(maybe).then(std::nullopt);
    using CW = decltype(cw);

    // OK
    static_assert(can_call_else_with<CW, decltype(bar.textN)>);
    static_assert(can_call_else_with<CW, decltype(foo.textNnD)>);
    static_assert(can_call_else_with<CW, decltype(std::optional<std::string>(
                                             std::nullopt))>);

    // Fail: the value type of CASE cannot be determined if all `then` and
    // `else_` are `nullopt`.
    static_assert(not can_call_else_with<CW, decltype(std::nullopt)>);

    // Fail: Anything that does not have a value.
    static_assert(
        not can_call_else_with<CW, decltype(bar.boolNn = true)>);
    static_assert(
        not can_call_else_with<CW, decltype(bar.boolNn.as(something))>);
    static_assert(not can_call_else_with<CW, decltype(bar)>);

    auto cw2 = cw.when(maybe).then(bar.textN).when(maybe);
    using CW2 = decltype(cw2);

    // OK
    static_assert(can_call_then_with<CW2, decltype(bar.textN)>);
    static_assert(can_call_then_with<CW2, decltype(foo.textNnD)>);
    static_assert(can_call_then_with<CW2, decltype(std::optional<std::string>(
                                             std::nullopt))>);

    // OK: the value type of CASE is determined by the first THEN expression.
    static_assert(can_call_then_with<CW2, decltype(std::nullopt)>);

    // Fail: Anything that does not the right value type:
    static_assert(
        not can_call_then_with<CW2, decltype(bar.boolNn)>);
    static_assert(
        not can_call_then_with<CW2, decltype(bar.intN)>);

    // Fail: Anything that does not have a value.
    static_assert(
        not can_call_then_with<CW2, decltype(bar.boolNn = true)>);
    static_assert(
        not can_call_then_with<CW2, decltype(bar.boolNn.as(something))>);
    static_assert(not can_call_then_with<CW2, decltype(bar)>);
  }
  // -----------------------
  // Cannot mix data types
  // -----------------------
  {
    auto cw = sqlpp::case_when(maybe).then(foo.intN);
    using CW = decltype(cw);
    auto cw2 = cw.when(maybe);
    using CW2 = decltype(cw2);

    // OK
    static_assert(can_call_else_with<CW, decltype(bar.intN)>);
    static_assert(can_call_else_with<CW, decltype(bar.id)>);
    static_assert(can_call_then_with<CW2, decltype(bar.intN)>);
    static_assert(can_call_then_with<CW2, decltype(bar.id)>);

    // Not OK
    static_assert(not can_call_else_with<CW, decltype(foo.uIntN)>);
    static_assert(not can_call_else_with<CW, decltype(foo.doubleN)>);
    static_assert(not can_call_then_with<CW2, decltype(foo.uIntN)>);
    static_assert(not can_call_then_with<CW2, decltype(foo.doubleN)>);
  }
  {
    auto cw = sqlpp::case_when(maybe).then(dt.dateN);
    using CW = decltype(cw);
    auto cw2 = cw.when(maybe);
    using CW2 = decltype(cw2);

    // OK
    static_assert(can_call_else_with<CW, decltype(dt.dateN)>);
    static_assert(can_call_then_with<CW2, decltype(dt.dateN)>);

    // Not OK
    static_assert(not can_call_else_with<CW, decltype(dt.timePointN)>);
    static_assert(not can_call_else_with<CW, decltype(dt.timeOfDayN)>);
    static_assert(not can_call_then_with<CW2, decltype(dt.timePointN)>);
    static_assert(not can_call_then_with<CW2, decltype(dt.timeOfDayN)>);
  }
}
