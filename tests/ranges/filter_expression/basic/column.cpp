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

#include <print>

#include <sqlpp26/core/type_traits.h>
#include <sqlpp26/ranges/aggregate_function/count.h>
#include <sqlpp26/ranges/aggregate_function/sum.h>
#include <sqlpp26/ranges/clause/from.h>
#include <sqlpp26/ranges/clause/group_by.h>
#include <sqlpp26/ranges/clause/insert.h>
#include <sqlpp26/ranges/clause/insert_value_list.h>
#include <sqlpp26/ranges/clause/into.h>
#include <sqlpp26/ranges/clause/select.h>
#include <sqlpp26/ranges/clause/select_column_list.h>
#include <sqlpp26/ranges/clause/single_table.h>
#include <sqlpp26/ranges/clause/update.h>
#include <sqlpp26/ranges/clause/update_set_list.h>
#include <sqlpp26/ranges/clause/where.h>
#include <sqlpp26/ranges/operator/as_expression.h>
#include <sqlpp26/ranges/operator/assign_expression.h>
#include <sqlpp26/ranges/operator/comparison_expression.h>
#include <sqlpp26/ranges/operator/logical_expression.h>
#include <sqlpp26/ranges/query/statement.h>
#include <sqlpp26/ranges/ranges.h>
#include <sqlpp26/ranges/to_filter_expression.h>

namespace test {
struct Foo {
  int id;
  std::string_view something;
};

struct _tab_foo {
  using generator = sqlpp::make_stl_table<_tab_foo, Foo>;
};
using tab_foo = sqlpp::table<_tab_foo>;
}

template <typename Statement>
constexpr void run(const Statement&) {
  consteval { Statement::check_basic_consistency(); }
}


int main() {
  auto tab_foo = test::tab_foo{};
  auto filter = to_filter_expression(tab_foo.id);
  constexpr auto foo = [tab_foo]() -> test::Foo{
    constexpr auto insert_set_expression = insert_into(tab_foo).set(tab_foo.id = 123, tab_foo.something = "cheese");
    run(insert_set_expression);
    constexpr auto insert_set_filter = to_filter_expression(insert_set_expression);

    std::vector<test::Foo> v;
    insert_set_filter.insert(v);
    auto& back = v.back();
    if (back.id != 123 or back.something != "cheese") { throw std::logic_error("unexpected values in back of vector after insert"); }

    constexpr auto update_set_expression = update(tab_foo).set(tab_foo.id = 1234, tab_foo.something = "cheesecake").where(tab_foo.id > 17);
    constexpr auto update_set_filter = to_filter_expression(update_set_expression);
    run(update_set_expression);
    update_set_filter.update(v);
    if (back.id != 1234 or back.something != "cheesecake") { throw std::logic_error("unexpected values in back of vector after update"); }

    return v.back();
  }();
  static_assert(foo.id ==1234);
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

  constexpr auto select_expression = select(tab_foo.id, tab_foo.something).from(tab_foo).where(tab_foo.id > 17);
  constexpr auto select_filter = to_filter_expression(select_expression);

  auto v = std::vector{foo};
  auto result = select_filter.select(v);
  if (result.empty()) { std::println("result unexpectedly empty"); throw 7;}
  for (const auto& row : result)
  {
    std::println("{},{}", row.id, row.something);
    if (row.id != 1234) { std::println("unexpected value for row.id: {}", row.id); throw 7;}
    if (row.something != "cheesecake") { std::println("unexpected value for row.something: {}", row.id); throw 7;}
  }

  v.push_back({1, "a"});
  v.push_back({1, "c"});
  v.push_back({1, "d"});
  v.push_back({1, "e"});

  v.push_back({3, "a"});
  v.push_back({3, "c"});
  v.push_back({3, "d"});

  const auto sum_ex = sum(tab_foo.id);
  const auto sum_as = sum_ex.as<"total">();
  const auto sum_filter = to_filter_expression(sum_ex);
  std::println("sum: {}", sum_filter.aggregate(v));

  const auto count_ex = count(tab_foo.id);
  const auto count_as = count_ex.as<"row_count">();
  const auto count_filter = to_filter_expression(count_ex);
  std::println("count: {}", count_filter.aggregate(v));

  constexpr auto group_by_expression =
      select(tab_foo.id, count(tab_foo.id).as<"row_count">(),
             sum(tab_foo.id).as<"total">())
          .from(tab_foo)
          .where(tab_foo.id != 17)
          .group_by(tab_foo.id);
  constexpr auto group_by_filter = to_filter_expression(group_by_expression);
  auto group_by_result = group_by_filter.select(v);
  for (const auto& row : group_by_result)
  {
    std::println("id: {}, row_count: {}, sum: {}", row.id, row.row_count, row.total);
  }
  /*
  std::vector<test::Foo> v; // This is a table object
        constexpr auto insert_set_expression = insert_into(tab_foo).set(tab_foo.id = 123, tab_foo.something = "cheese");
      constexpr auto insert_set_filter = to_filter_expression(insert_set_expression);
      */


}
