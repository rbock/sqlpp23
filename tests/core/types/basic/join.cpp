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

void test_join() {
  auto foo = test::tab_foo{};
  auto bar = test::tab_bar{};
  auto cheese = foo.as<"cheese">();
  auto cake = foo.as<"cake">();
  auto verb = sqlpp::verbatim_table("verb").as<"verb">();
  auto cte = sqlpp::cte<"CTE">().as(select(foo.id).from(foo));
  auto sel_as = select(all_of(foo))
                    .from(foo)
                    .where(foo.id == sqlpp::parameter(foo.id))
                    .as<"sel_as">();

  using Foo = decltype(foo);
  using Bar = decltype(bar);
  using Cheese = decltype(cheese);
  using Cake = decltype(cake);
  using Verb = decltype(verb);
  using CteRef = sqlpp::cte_ref_t<"CTE">;
  using SelAsRef = sqlpp::select_ref_t<"sel_as">;

  // Pre-join
  static_assert(not sqlpp::is_table<decltype(foo.join(bar))>::value, "");

  // Join of tables
  {
    using J = decltype(foo.join(bar).on(foo.id == bar.id));
    static_assert(sqlpp::is_table<J>::value, "");
    static_assert(sqlpp::provided_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<Foo, Bar>());
    static_assert(sqlpp::provided_static_tables_of<J>::func() ==
                  sqlpp::provided_tables_of<J>::func());
    static_assert(sqlpp::provided_optional_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<>());
  }

  {
    using J = decltype(foo.cross_join(bar));
    static_assert(sqlpp::is_table<J>::value, "");
    static_assert(sqlpp::provided_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<Foo, Bar>());
    static_assert(sqlpp::provided_static_tables_of<J>::func() ==
                  sqlpp::provided_tables_of<J>::func());
    static_assert(sqlpp::provided_optional_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<>());
  }

  {
    using J = decltype(foo.inner_join(bar).on(foo.id == bar.id));
    static_assert(sqlpp::is_table<J>::value, "");
    static_assert(sqlpp::provided_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<Foo, Bar>());
    static_assert(sqlpp::provided_static_tables_of<J>::func() ==
                  sqlpp::provided_tables_of<J>::func());
    static_assert(sqlpp::provided_optional_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<>());
  }

  {
    using J = decltype(foo.left_outer_join(bar).on(foo.id == bar.id));
    static_assert(sqlpp::is_table<J>::value, "");
    static_assert(sqlpp::provided_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<Foo, Bar>());
    static_assert(sqlpp::provided_static_tables_of<J>::func() ==
                  sqlpp::provided_tables_of<J>::func());
    static_assert(sqlpp::provided_optional_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<Bar>());
  }

  {
    using J = decltype(foo.right_outer_join(bar).on(foo.id == bar.id));
    static_assert(sqlpp::is_table<J>::value, "");
    static_assert(sqlpp::provided_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<Foo, Bar>());
    static_assert(sqlpp::provided_static_tables_of<J>::func() ==
                  sqlpp::provided_tables_of<J>::func());
    static_assert(sqlpp::provided_optional_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<Foo>());
  }

  {
    using J = decltype(foo.full_outer_join(bar).on(foo.id == bar.id));
    static_assert(sqlpp::is_table<J>::value, "");
    static_assert(sqlpp::provided_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<Foo, Bar>());
    static_assert(sqlpp::provided_static_tables_of<J>::func() ==
                  sqlpp::provided_tables_of<J>::func());
    static_assert(sqlpp::provided_optional_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<Foo, Bar>());
  }

  // Join with rhs alias table
  {
    using J = decltype(foo.join(cheese).on(foo.id == cheese.id));
    static_assert(sqlpp::is_table<J>::value, "");
    static_assert(sqlpp::provided_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<Foo, Cheese>());
    static_assert(sqlpp::provided_static_tables_of<J>::func() ==
                  sqlpp::provided_tables_of<J>::func());
    static_assert(sqlpp::provided_optional_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<>());
  }

  // Join with rhs alias table
  {
    using J = decltype(foo.join(dynamic(true, cheese)).on(foo.id == cheese.id));
    static_assert(sqlpp::is_table<J>::value, "");
    static_assert(sqlpp::provided_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<Foo, Cheese>());
    static_assert(sqlpp::provided_static_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<Foo>());
    static_assert(sqlpp::provided_optional_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<>());
  }

  // Join with two alias tables
  {
    using J = decltype(cheese.join(cake).on(cheese.id == cake.id));
    static_assert(sqlpp::is_table<J>::value, "");
    static_assert(sqlpp::provided_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<Cheese, Cake>());
    static_assert(sqlpp::provided_static_tables_of<J>::func() ==
                  sqlpp::provided_tables_of<J>::func());
    static_assert(sqlpp::provided_optional_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<>());
  }

  // Join with verbatim table
  {
    using J = decltype(verb.join(cake).on(
        sqlpp::verbatim<sqlpp::integral>("verb.id") == cake.id));
    static_assert(sqlpp::is_table<J>::value, "");
    static_assert(sqlpp::provided_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<Verb, Cake>());
    static_assert(sqlpp::provided_static_tables_of<J>::func() ==
                  sqlpp::provided_tables_of<J>::func());
    static_assert(sqlpp::provided_optional_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<>());
  }

  // Join with select as
  {
    using J = decltype(sel_as.join(foo).on(sel_as.id == foo.id));
    static_assert(sqlpp::is_table<J>::value, "");
    static_assert(sqlpp::provided_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<SelAsRef, Foo>());
    static_assert(sqlpp::provided_static_tables_of<J>::func() ==
                  sqlpp::provided_tables_of<J>::func());
    static_assert(sqlpp::provided_optional_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<>());
  }

  // Join with select as and parameters
  {
    using J = decltype(sel_as.join(foo).on(
        sel_as.id == foo.id + sqlpp::parameter<"a", int64_t>()));
    static_assert(sqlpp::is_table<J>::value, "");
    static_assert(sqlpp::provided_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<SelAsRef, Foo>());
    static_assert(sqlpp::provided_static_tables_of<J>::func() ==
                  sqlpp::provided_tables_of<J>::func());
    static_assert(sqlpp::provided_optional_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<>());

    // parameters from sub select and condition are being exposed
    using ExpectedParameters =
        sqlpp::detail::type_vector<sqlpp::parameter_t<int64_t, "id">,
                                   sqlpp::parameter_t<int64_t, "a">>;
    static_assert(
        std::is_same<sqlpp::parameters_of_t<J>, ExpectedParameters>::value, "");
  }

  // Join with cte
  {
    using J = decltype(cte.join(cake).on(cte.id == cake.id));
    static_assert(sqlpp::is_table<J>::value, "");
    static_assert(sqlpp::provided_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<CteRef, Cake>());
    static_assert(sqlpp::provided_static_tables_of<J>::func() ==
                  sqlpp::provided_tables_of<J>::func());
    static_assert(sqlpp::provided_optional_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<>());
    static_assert(sqlpp::required_ctes_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<CteRef>());
  }

  // Join with dynamic table
  {
    using J = decltype(foo.join(dynamic(true, bar)).on(foo.id == bar.id));
    static_assert(sqlpp::is_table<J>::value, "");
    static_assert(sqlpp::provided_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<Foo, Bar>());
    static_assert(sqlpp::provided_static_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<Foo>());
    static_assert(sqlpp::provided_optional_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<>());
  }

  // Join with dynamic table and static table
  {
    using J = decltype(foo.cross_join(dynamic(true, bar))
                           .join(cheese)
                           .on(foo.id == cheese.id and
                               dynamic(true, bar.id == cheese.id)));
    static_assert(sqlpp::is_table<J>::value, "");
    static_assert(sqlpp::provided_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<Foo, Bar, Cheese>());
    static_assert(sqlpp::provided_static_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<Foo, Cheese>());
    static_assert(sqlpp::provided_optional_tables_of<J>::func() ==
                  sqlpp::detail::make_type_info_set<>());
  }
}

int main() {
  void test_join();
}
