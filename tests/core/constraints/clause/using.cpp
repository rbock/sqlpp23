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
concept can_call_using_with_standalone =
    requires(Expressions... expressions) { sqlpp::using_(expressions...); };
template <typename... Expressions>
concept can_call_using_with_in_statement =
    requires(Expressions... expressions) {
      sqlpp::statement_t<sqlpp::no_using_t>{}.using_(expressions...);
    };

template <typename... Expressions>
concept can_call_using_with = can_call_using_with_standalone<Expressions...> and
                              can_call_using_with_in_statement<Expressions...>;

template <typename... Expressions>
concept cannot_call_using_with =
    not(can_call_using_with_standalone<Expressions...> or
        can_call_using_with_in_statement<Expressions...>);
}  // namespace

int main() {
  const auto maybe = true;
  const auto foo = test::tab_foo{};
  const auto bar = test::tab_bar{};
  const auto c = sqlpp::cte<"something">().as(select(foo.id).from(foo));

  // using_(<non arguments>) is inconsistent and cannot be constructed.
  static_assert(cannot_call_using_with<>, "");

  // using_(<non table>) cannot be called.
  static_assert(cannot_call_using_with<decltype(foo.id)>, "");
  static_assert(cannot_call_using_with<decltype(dynamic(true, foo.id))>, "");

  // using_(<table or join>) can be called, even if it is dynamic (we might not
  // need the using).
  static_assert(can_call_using_with<decltype(foo)>, "");
  static_assert(can_call_using_with<decltype(foo.cross_join(bar))>, "");
  static_assert(can_call_using_with<decltype(c)>, "");

  static_assert(can_call_using_with<decltype(dynamic(maybe, foo))>, "");
  static_assert(
      can_call_using_with<decltype(dynamic(maybe, foo.cross_join(bar)))>, "");
  static_assert(can_call_using_with<decltype(dynamic(maybe, c))>, "");

  // using_ is not required
  {
    auto s = sqlpp::statement_t<sqlpp::no_using_t>{};
    using S = decltype(s);
    expect_basic_consistency_succeeds<S>();
    expect_prepare_consistency_succeeds<S>();
  }

  // using_ must not require unknown ctes
  {
    auto s = with(dynamic(true, c)) << using_(c);
    using S = decltype(s);
    expect_basic_consistency_fails<
        S,
        "The using-clause statically requires cte something which is only "
        "known dynamically in the statement">();
  }

  {
    auto s = using_(c);
    using S = decltype(s);
    expect_basic_consistency_fails<S,
                                   "The using-clause requires cte something "
                                   "which is not known in the statement">();
  }

  // using_ must not repeat a table from another clause
  {
    auto s = delete_from(foo) << using_(foo);
    using S = decltype(s);
    expect_basic_consistency_fails<
        S, "Table(s) of name tab_foo provided twice in the statement">();
  }
  {
    auto s = delete_from(foo) << using_(dynamic(true, foo));
    using S = decltype(s);
    expect_basic_consistency_fails<
        S, "Table(s) of name tab_foo provided twice in the statement">();
  }
  {
    auto s = delete_from(foo) << using_(bar.as<"tab_foo">());
    using S = decltype(s);
    expect_basic_consistency_fails<
        S, "Table(s) of name tab_foo provided twice in the statement">();
  }
}
