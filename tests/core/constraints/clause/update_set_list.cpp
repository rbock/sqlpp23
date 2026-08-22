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
template <typename... Expressions>
concept can_call_update_set_with_standalone =
    requires(Expressions... expressions) { sqlpp::update_set(expressions...); };
template <typename... Expressions>
concept can_call_update_set_with_in_statement =
    requires(Expressions... expressions) {
      sqlpp::statement_t<sqlpp::no_update_set_list_t>{}.set(expressions...);
    };

template <typename... Expressions>
concept can_call_update_set_with =
    can_call_update_set_with_standalone<Expressions...> and
    can_call_update_set_with_in_statement<Expressions...>;

template <typename... Expressions>
concept cannot_call_update_set_with =
    not(can_call_update_set_with_standalone<Expressions...> or
        can_call_update_set_with_in_statement<Expressions...>);
}  // namespace

int main() {
  const auto maybe = true;
  const auto bar = test::tab_bar{};
  const auto foo = test::tab_foo{};

  // OK
  update_set(bar.bool_nn = true);
  update_set(bar.bool_nn = true, bar.text_n = "");

  update_set(dynamic(maybe, bar.bool_nn = true));
  update_set(dynamic(maybe, bar.bool_nn = true), bar.text_n = "");
  update_set(bar.bool_nn = true, dynamic(maybe, bar.text_n = ""));
  update_set(dynamic(maybe, bar.bool_nn = true), dynamic(maybe, bar.text_n = ""));

  // Cannot update nothing
  static_assert(cannot_call_update_set_with<>);

  // update_set requires assignments as arguments and cannot be called with
  // anything else.
  static_assert(can_call_update_set_with<decltype(bar.bool_nn = true)>,
                "OK, argument is an assignment");
  static_assert(cannot_call_update_set_with<decltype(bar.id == 7)>,
                "not an assignment: comparison");
  static_assert(
      cannot_call_update_set_with<decltype(bar.int_n = 7), decltype(bar.bool_nn)>,
      "not an assignment: bool_nn");

  // Try duplicate columns
  static_assert(cannot_call_update_set_with<decltype(bar.bool_nn = true),
                                            decltype(bar.bool_nn = false)>);
  static_assert(cannot_call_update_set_with<decltype(bar.bool_nn = true),
                                            decltype(bar.text_n = ""),
                                            decltype(bar.bool_nn = false)>);
  static_assert(
      cannot_call_update_set_with<decltype(dynamic(maybe, bar.bool_nn = true)),
                                  decltype(bar.bool_nn = false)>);
  static_assert(cannot_call_update_set_with<decltype(bar.bool_nn = true),
                                            decltype(dynamic(
                                                maybe, bar.bool_nn = false))>);
  static_assert(cannot_call_update_set_with<
                decltype(dynamic(maybe, bar.bool_nn = true)),
                decltype(dynamic(maybe, bar.bool_nn = false))>);

  // Try to update multiple tables at once
  static_assert(cannot_call_update_set_with<decltype(bar.bool_nn = true),
                                            decltype(foo.float_n = 7)>);
  static_assert(
      cannot_call_update_set_with<decltype(dynamic(maybe, bar.bool_nn = true)),
                                  decltype(foo.float_n = 7)>);
  static_assert(
      cannot_call_update_set_with<decltype(bar.bool_nn = true),
                                  decltype(dynamic(maybe, foo.float_n = 7))>);
  static_assert(
      cannot_call_update_set_with<decltype(dynamic(maybe, bar.bool_nn = true)),
                                  decltype(dynamic(maybe, foo.float_n = 7))>);

  {
    auto u = update(bar);
    using U = decltype(u);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<U>,
                               sqlpp::assert_update_assignments_t>::value,
                  "");
  }

  {
    auto u = update(bar).set(foo.int_n = 7);
    using U = decltype(u);
    static_assert(
        std::is_same<
            sqlpp::statement_consistency_check_t<U>,
            sqlpp::assert_no_unknown_tables_in_update_assignments_t>::value,
        "");
  }
}
