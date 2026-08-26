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

void test_column() {
  {
    // Column integer with default (auto-increment).
    auto foo = test::tab_foo{};
    using Foo = decltype(foo);
    using Id = decltype(foo.id);
    using Cheese = decltype(foo.id.as<"cheese">());
    using Bar = decltype(foo.as<"tab_bar">());
    using BarId = decltype(foo.as<"tab_bar">().id);
    using BarCheese = decltype(foo.as<"tab_bar">().id.as<"cheese">());

    static_assert(not sqlpp::is_table<Id>::value, "");
    static_assert(sqlpp::has_default<Id>::value, "");

    // Columns are aggregates when in group by, otherwise they are
    // non-aggregates. But they are never neutral.
    static_assert(not sqlpp::is_aggregate_neutral<Id>::value, "");

    static_assert(std::string_view(sqlpp::name_of_v<Id>) == "id");
    static_assert(sqlpp::provided_tables_of<Id>::func() ==
                  sqlpp::detail::make_type_info_set<>());
    static_assert(sqlpp::provided_static_tables_of<Id>::func() ==
                  sqlpp::provided_tables_of<Id>::func());
    static_assert(sqlpp::provided_optional_tables_of<Id>::func() ==
                  sqlpp::provided_tables_of<Id>::func());
    static_assert(sqlpp::required_tables_of<Id>::func() ==
                  sqlpp::detail::make_type_info_set<Foo>());
    static_assert(sqlpp::required_static_tables_of<Id>::func() ==
                  sqlpp::required_tables_of<Id>::func());

    static_assert(
        std::is_same<sqlpp::data_type_of_t<Id>, sqlpp::integral>::value);

    // tab_foo.id AS cheese
    // This is only useful SELECT. It therefore exposes no value directly.
    // It does require its table, though.
    static_assert(not sqlpp::is_table<Cheese>::value, "");
    static_assert(not sqlpp::has_default<Cheese>::value, "");

    static_assert(std::string_view(sqlpp::name_of_v<Cheese>) == "cheese");
    static_assert(sqlpp::provided_tables_of<Cheese>::func() ==
                  sqlpp::detail::make_type_info_set<>());
    static_assert(sqlpp::provided_static_tables_of<Cheese>::func() ==
                  sqlpp::provided_tables_of<Cheese>::func());
    static_assert(sqlpp::provided_optional_tables_of<Cheese>::func() ==
                  sqlpp::provided_tables_of<Cheese>::func());
    static_assert(sqlpp::required_tables_of<Cheese>::func() ==
                  sqlpp::detail::make_type_info_set<Foo>());
    static_assert(sqlpp::required_static_tables_of<Cheese>::func() ==
                  sqlpp::required_tables_of<Cheese>::func());

    static_assert(
        std::is_same<sqlpp::data_type_of_t<Cheese>, sqlpp::no_value_t>::value);

    // (tab_foo AS bar).id
    static_assert(not sqlpp::is_table<BarId>::value, "");
    static_assert(sqlpp::has_default<BarId>::value, "");

    static_assert(std::string_view(sqlpp::name_of_v<BarId>) == "id");
    static_assert(sqlpp::provided_tables_of<BarId>::func() ==
                  sqlpp::detail::make_type_info_set<>());
    static_assert(sqlpp::provided_static_tables_of<BarId>::func() ==
                  sqlpp::provided_tables_of<BarId>::func());
    static_assert(sqlpp::provided_optional_tables_of<BarId>::func() ==
                  sqlpp::provided_tables_of<BarId>::func());
    static_assert(sqlpp::required_tables_of<BarId>::func() ==
                  sqlpp::detail::make_type_info_set<Bar>());
    static_assert(sqlpp::required_static_tables_of<BarId>::func() ==
                  sqlpp::required_tables_of<BarId>::func());

    static_assert(std::is_same_v<sqlpp::data_type_of_t<BarId>, int64_t >, "");

    // (tab_foo as bar).id.as(cheese)
    static_assert(not sqlpp::is_table<BarCheese>::value, "");
    static_assert(not sqlpp::has_default<BarCheese>::value, "");

    static_assert(std::string_view(sqlpp::name_of_v<BarCheese>) == "cheese");
    static_assert(sqlpp::provided_tables_of<BarCheese>::func() ==
                  sqlpp::detail::make_type_info_set<>());
    static_assert(sqlpp::provided_static_tables_of<BarCheese>::func() ==
                  sqlpp::provided_tables_of<BarCheese>::func());
    static_assert(sqlpp::provided_optional_tables_of<BarCheese>::func() ==
                  sqlpp::provided_tables_of<BarCheese>::func());
    static_assert(sqlpp::required_tables_of<BarCheese>::func() ==
                  sqlpp::required_tables_of<BarId>::func());
    static_assert(sqlpp::required_static_tables_of<BarCheese>::func() ==
                  sqlpp::required_tables_of<BarCheese>::func());

    static_assert(
        std::is_same_v<sqlpp::data_type_of_t<BarCheese>, sqlpp::no_value_t>);
  }

  {
    // Column optional (can be null) text with default.
    auto bar = test::tab_bar{};
    using Bar = decltype(bar);
    using TextN = decltype(bar.text_n);

    static_assert(not sqlpp::is_table<TextN>::value, "");
    static_assert(sqlpp::has_default<TextN>::value, "");

    static_assert(std::string_view(sqlpp::name_of_v<TextN>) == "text_n");
    static_assert(sqlpp::provided_tables_of<TextN>::func() ==
                  sqlpp::detail::make_type_info_set<>());
    static_assert(sqlpp::provided_static_tables_of<TextN>::func() ==
                  sqlpp::provided_tables_of<TextN>::func());
    static_assert(sqlpp::provided_optional_tables_of<TextN>::func() ==
                  sqlpp::provided_tables_of<TextN>::func());
    static_assert(sqlpp::required_tables_of<TextN>::func() ==
                  sqlpp::detail::make_type_info_set<Bar>());
    static_assert(sqlpp::required_static_tables_of<TextN>::func() ==
                  sqlpp::required_tables_of<TextN>::func());

    static_assert(std::is_same_v<sqlpp::data_type_of_t<TextN>,
                                 std::optional<std::string_view>>);
  }

  {
    // Column bool without default.
    auto bar = test::tab_bar{};
    using Bar = decltype(bar);
    using BoolNn = decltype(bar.bool_nn);

    static_assert(not sqlpp::is_table<BoolNn>::value, "");
    static_assert(not sqlpp::has_default<BoolNn>::value, "");

    static_assert(std::string_view(sqlpp::name_of_v<BoolNn>) == "bool_nn");
    static_assert(sqlpp::provided_tables_of<BoolNn>::func() ==
                  sqlpp::detail::make_type_info_set<>());
    static_assert(sqlpp::provided_static_tables_of<BoolNn>::func() ==
                  sqlpp::provided_tables_of<BoolNn>::func());
    static_assert(sqlpp::provided_optional_tables_of<BoolNn>::func() ==
                  sqlpp::provided_tables_of<BoolNn>::func());
    static_assert(sqlpp::required_tables_of<BoolNn>::func() ==
                  sqlpp::detail::make_type_info_set<Bar>());
    static_assert(sqlpp::required_static_tables_of<BoolNn>::func() ==
                  sqlpp::required_tables_of<BoolNn>::func());

    static_assert(
        std::is_same<sqlpp::data_type_of_t<BoolNn>, sqlpp::boolean>::value);
  }

  {
    // table() function
    auto bar = test::tab_bar{};
    using Bar = decltype(bar);
    using Id = decltype(bar.id);

    static_assert(std::is_same<sqlpp::table_of_t<Id>, Bar>::value);
  }
}

int main() {
  void test_column();
}
