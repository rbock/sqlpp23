/*
 * Copyright (c) 2026, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
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

#include <sqlpp26/ranges/to_filter_expression.h>

#include <sqlpp26/core/type_traits.h>
#include <sqlpp26/ranges/clause/insert.h>
#include <sqlpp26/ranges/clause/into.h>
#include <sqlpp26/ranges/clause/insert_value_list.h>
#include <sqlpp26/ranges/clause/update_set_list.h>
#include <sqlpp26/ranges/clause/select_column_list.h>
#include <sqlpp26/ranges/operator/assign_expression.h>
#include <sqlpp26/ranges/operator/comparison_expression.h>
#include <sqlpp26/ranges/operator/logical_expression.h>
#include <sqlpp26/ranges/query/statement.h>
#include <sqlpp26/ranges/ranges.h>

namespace test {
struct Foo {
  int id;
  std::string_view something;
};

struct _TabFoo {
  using generator = sqlpp::make_stl_table<_TabFoo, Foo>;
};
using TabFoo = sqlpp::table<_TabFoo>;
}

template <typename Statement>
constexpr void run(const Statement&) {
  consteval { Statement::check_basic_consistency(); }
}


int main() {
  auto tab_foo = test::TabFoo{};
  auto filter = to_filter_expression(tab_foo.id);
  constexpr auto foo = [tab_foo]() {
    constexpr auto insert_set_expression = insert_into(tab_foo).set(tab_foo.id = 123, tab_foo.something = "cheese");
    run(insert_set_expression);
    constexpr auto insert_set_filter = to_filter_expression(insert_set_expression);
    auto a_foo = insert_set_filter(test::Foo{});
    if (a_foo.id != 123 or a_foo.something != "cheese") { throw a_foo; }

    constexpr auto update_set_expression = update_set(tab_foo.id = 1234, tab_foo.something = "cheesecake");
    constexpr auto update_set_filter = to_filter_expression(update_set_expression);
    //run(update_set_expression);
    return update_set_filter(a_foo);
  }();
  constexpr auto resultId = filter(foo);
  static_assert(resultId == foo.id);

  constexpr auto value_filter = sqlpp::to_filter_expression(174);
  static_assert(value_filter(foo) == 174);

  constexpr auto less_expression = tab_foo.id < 12345;
  constexpr auto less_filter = to_filter_expression(less_expression);

  static_assert(less_filter(foo));

  constexpr auto and_expression = tab_foo.id < 12345 and tab_foo.id > 17;
  constexpr auto and_filter = to_filter_expression(and_expression);

  static_assert(and_filter(foo));

  constexpr auto select_expression = select_columns(tab_foo.id, tab_foo.something);
  constexpr auto select_filter = to_filter_expression(select_expression);

  constexpr auto result = select_filter(foo);

  static_assert(result.id == 1234);
  static_assert(result.something == "cheesecake");

}
