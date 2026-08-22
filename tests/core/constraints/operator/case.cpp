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
  const auto foo = test::tab_foo{};
  const auto bar = test::tab_bar{};
  const auto dt = test::tab_date_time{};

  // -----------------------
  // case_when()
  // -----------------------

  // OK
  static_assert(can_call_case_when_with<decltype(true)>);
  static_assert(
      can_call_case_when_with<decltype(std::optional{true})>);
  static_assert(can_call_case_when_with<decltype(foo.bool_n)>);
  static_assert(can_call_case_when_with<decltype(bar.bool_nn)>);
  static_assert(can_call_case_when_with<decltype(bar.bool_nn == true)>);
  static_assert(can_call_case_when_with<decltype(count(foo.id) > 0)>);
  static_assert(can_call_case_when_with<decltype(std::nullopt)>);

  // Fail: Cannot call case_when with renamed boolean
  static_assert(
      not can_call_case_when_with<decltype(bar.bool_nn.as<"something">())>);
  // Fail: Cannot call case_when with non-boolean expressions.
  static_assert(not can_call_case_when_with<decltype(bar.id)>);
  static_assert(not can_call_case_when_with<decltype(bar.bool_nn = true)>);
  static_assert(not can_call_case_when_with<decltype(bar)>);

  // -----------------------
  // case_when.then()
  // -----------------------
  {
    auto cw = sqlpp::case_when(maybe);
    using CW = decltype(cw);

    // OK
    static_assert(can_call_then_with<CW, decltype(bar.id)>);
    static_assert(can_call_then_with<CW, decltype(bar.text_n)>);
    static_assert(
        can_call_then_with<CW,
                           decltype(std::optional<int>(std::nullopt))>);
    // OK, can call `then` with naked `nullopt`, but will need a non-nullopt `then` or `else_` later.
    static_assert(can_call_then_with<CW, decltype(std::nullopt)>);

    // Fail: Anything that does not have a value.
    static_assert(
        not can_call_then_with<CW, decltype(bar.bool_nn = true)>);
    static_assert(
        not can_call_then_with<CW, decltype(bar.bool_nn.as<"something">())>);
    static_assert(not can_call_then_with<CW, decltype(bar)>);
  }

  // -----------------------
  // case_when.then(<nullable text>)
  // -----------------------
  {
    auto cw = sqlpp::case_when(maybe).then(bar.text_n);
    using CW = decltype(cw);

    // OK
    static_assert(can_call_else_with<CW, decltype(bar.text_n)>);
    static_assert(can_call_else_with<CW, decltype(foo.text_nn_d)>);
    static_assert(can_call_else_with<CW, decltype(std::optional<std::string>(
                                             std::nullopt))>);

    // OK: the value type of CASE is determined by the THEN expression.
    static_assert(can_call_else_with<CW, decltype(std::nullopt)>);

    // Fail: Anything that does not the right value type:
    static_assert(
        not can_call_else_with<CW, decltype(bar.bool_nn)>);
    static_assert(
        not can_call_else_with<CW, decltype(bar.int_n)>);

    // Fail: Anything that does not have a value.
    static_assert(
        not can_call_else_with<CW, decltype(bar.bool_nn = true)>);
    static_assert(
        not can_call_else_with<CW, decltype(bar.bool_nn.as<"something">())>);
    static_assert(not can_call_else_with<CW, decltype(bar)>);

    auto cw2 = sqlpp::case_when(maybe).then(bar.text_n).when(maybe);
    using CW2 = decltype(cw2);

    // OK
    static_assert(can_call_then_with<CW2, decltype(bar.text_n)>);
    static_assert(can_call_then_with<CW2, decltype(foo.text_nn_d)>);
    static_assert(can_call_then_with<CW2, decltype(std::optional<std::string>(
                                             std::nullopt))>);

    // OK: the value type of CASE is determined by the first THEN expression.
    static_assert(can_call_then_with<CW2, decltype(std::nullopt)>);

    // Fail: Anything that does not the right value type:
    static_assert(
        not can_call_then_with<CW2, decltype(bar.bool_nn)>);
    static_assert(
        not can_call_then_with<CW2, decltype(bar.int_n)>);

    // Fail: Anything that does not have a value.
    static_assert(
        not can_call_then_with<CW2, decltype(bar.bool_nn = true)>);
    static_assert(
        not can_call_then_with<CW2, decltype(bar.bool_nn.as<"something">())>);
    static_assert(not can_call_then_with<CW2, decltype(bar)>);
  }

  // -----------------------
  // case_when.then(integral).else_()
  // -----------------------
  {
    auto cw = sqlpp::case_when(maybe).then(bar.text_n);
    using CW = decltype(cw);

    // OK
    static_assert(can_call_else_with<CW, decltype(bar.text_n)>);
    static_assert(can_call_else_with<CW, decltype(foo.text_nn_d)>);
    static_assert(can_call_else_with<CW, decltype(std::optional<std::string>(
                                             std::nullopt))>);

    // OK: the value type of CASE is determined by the THEN expression.
    static_assert(can_call_else_with<CW, decltype(std::nullopt)>);

    // Fail: Anything that does not the right value type:
    static_assert(
        not can_call_else_with<CW, decltype(bar.bool_nn)>);
    static_assert(
        not can_call_else_with<CW, decltype(bar.int_n)>);

    // Fail: Anything that does not have a value.
    static_assert(
        not can_call_else_with<CW, decltype(bar.bool_nn = true)>);
    static_assert(
        not can_call_else_with<CW, decltype(bar.bool_nn.as<"something">())>);
    static_assert(not can_call_else_with<CW, decltype(bar)>);

    auto cw2 = cw.when(maybe);
    using CW2 = decltype(cw2);

    // OK
    static_assert(can_call_then_with<CW2, decltype(bar.text_n)>);
    static_assert(can_call_then_with<CW2, decltype(foo.text_nn_d)>);
    static_assert(can_call_then_with<CW2, decltype(std::optional<std::string>(
                                             std::nullopt))>);

    // OK: the value type of CASE is determined by the first THEN expression.
    static_assert(can_call_then_with<CW2, decltype(std::nullopt)>);

    // Fail: Anything that does not the right value type:
    static_assert(
        not can_call_then_with<CW2, decltype(bar.bool_nn)>);
    static_assert(
        not can_call_then_with<CW2, decltype(bar.int_n)>);

    // Fail: Anything that does not have a value.
    static_assert(
        not can_call_then_with<CW2, decltype(bar.bool_nn = true)>);
    static_assert(
        not can_call_then_with<CW2, decltype(bar.bool_nn.as<"something">())>);
    static_assert(not can_call_then_with<CW2, decltype(bar)>);
  }

  // -----------------------
  // case_when.then(<non-nullable text>).else_()
  // -----------------------
  {
    auto cw = sqlpp::case_when(maybe).then(foo.text_nn_d);
    using CW = decltype(cw);

    // OK
    static_assert(can_call_else_with<CW, decltype(bar.text_n)>);
    static_assert(can_call_else_with<CW, decltype(foo.text_nn_d)>);
    static_assert(can_call_else_with<CW, decltype(std::optional<std::string>(
                                             std::nullopt))>);

    // OK: the value type of CASE is determined by the THEN expression.
    static_assert(can_call_else_with<CW, decltype(std::nullopt)>);

    // Fail: Anything that does not have a value.
    static_assert(
        not can_call_else_with<CW, decltype(bar.bool_nn = true)>);
    static_assert(
        not can_call_else_with<CW, decltype(bar.bool_nn.as<"something">())>);
    static_assert(not can_call_else_with<CW, decltype(bar)>);

    auto cw2 = cw.when(maybe).then(bar.text_n).when(maybe);
    using CW2 = decltype(cw2);

    // OK
    static_assert(can_call_then_with<CW2, decltype(bar.text_n)>);
    static_assert(can_call_then_with<CW2, decltype(foo.text_nn_d)>);
    static_assert(can_call_then_with<CW2, decltype(std::optional<std::string>(
                                             std::nullopt))>);

    // OK: the value type of CASE is determined by the first THEN expression.
    static_assert(can_call_then_with<CW2, decltype(std::nullopt)>);

    // Fail: Anything that does not the right value type:
    static_assert(
        not can_call_then_with<CW2, decltype(bar.bool_nn)>);
    static_assert(
        not can_call_then_with<CW2, decltype(bar.int_n)>);

    // Fail: Anything that does not have a value.
    static_assert(
        not can_call_then_with<CW2, decltype(bar.bool_nn = true)>);
    static_assert(
        not can_call_then_with<CW2, decltype(bar.bool_nn.as<"something">())>);
    static_assert(not can_call_then_with<CW2, decltype(bar)>);
  }

  // -----------------------
  // case_when.then(nullopt)
  // -----------------------
  {
    auto cw = sqlpp::case_when(maybe).then(std::nullopt);
    using CW = decltype(cw);

    // OK
    static_assert(can_call_else_with<CW, decltype(bar.text_n)>);
    static_assert(can_call_else_with<CW, decltype(foo.text_nn_d)>);
    static_assert(can_call_else_with<CW, decltype(std::optional<std::string>(
                                             std::nullopt))>);

    // Fail: the value type of CASE cannot be determined if all `then` and
    // `else_` are `nullopt`.
    static_assert(not can_call_else_with<CW, decltype(std::nullopt)>);

    // Fail: Anything that does not have a value.
    static_assert(
        not can_call_else_with<CW, decltype(bar.bool_nn = true)>);
    static_assert(
        not can_call_else_with<CW, decltype(bar.bool_nn.as<"something">())>);
    static_assert(not can_call_else_with<CW, decltype(bar)>);

    auto cw2 = cw.when(maybe).then(bar.text_n).when(maybe);
    using CW2 = decltype(cw2);

    // OK
    static_assert(can_call_then_with<CW2, decltype(bar.text_n)>);
    static_assert(can_call_then_with<CW2, decltype(foo.text_nn_d)>);
    static_assert(can_call_then_with<CW2, decltype(std::optional<std::string>(
                                             std::nullopt))>);
    static_assert(can_call_then_with<CW2, decltype(std::nullopt)>);

    // OK: the value type of CASE is determined by the first THEN expression.
    static_assert(can_call_then_with<CW2, decltype(std::nullopt)>);

    // Fail: Anything that does not the right value type:
    static_assert(
        not can_call_then_with<CW2, decltype(bar.bool_nn)>);
    static_assert(
        not can_call_then_with<CW2, decltype(bar.int_n)>);

    // Fail: Anything that does not have a value.
    static_assert(
        not can_call_then_with<CW2, decltype(bar.bool_nn = true)>);
    static_assert(
        not can_call_then_with<CW2, decltype(bar.bool_nn.as<"something">())>);
    static_assert(not can_call_then_with<CW2, decltype(bar)>);
  }
  // -----------------------
  // Cannot mix data types
  // -----------------------
  {
    auto cw = sqlpp::case_when(maybe).then(foo.int_n);
    using CW = decltype(cw);
    auto cw2 = cw.when(maybe);
    using CW2 = decltype(cw2);

    // OK
    static_assert(can_call_else_with<CW, decltype(bar.int_n)>);
    static_assert(can_call_else_with<CW, decltype(bar.id)>);
    static_assert(can_call_then_with<CW2, decltype(bar.int_n)>);
    static_assert(can_call_then_with<CW2, decltype(bar.id)>);

    // Not OK
    static_assert(not can_call_else_with<CW, decltype(foo.u_int_n)>);
    static_assert(not can_call_else_with<CW, decltype(foo.float_n)>);
    static_assert(not can_call_then_with<CW2, decltype(foo.u_int_n)>);
    static_assert(not can_call_then_with<CW2, decltype(foo.float_n)>);
  }
  {
    auto cw = sqlpp::case_when(maybe).then(dt.date_n);
    using CW = decltype(cw);
    auto cw2 = cw.when(maybe);
    using CW2 = decltype(cw2);

    // OK
    static_assert(can_call_else_with<CW, decltype(dt.date_n)>);
    static_assert(can_call_then_with<CW2, decltype(dt.date_n)>);

    // Not OK
    static_assert(not can_call_else_with<CW, decltype(dt.timestamp_n)>);
    static_assert(not can_call_else_with<CW, decltype(dt.time_n)>);
    static_assert(not can_call_then_with<CW2, decltype(dt.timestamp_n)>);
    static_assert(not can_call_then_with<CW2, decltype(dt.time_n)>);
  }
}
