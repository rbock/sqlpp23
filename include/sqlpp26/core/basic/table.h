#pragma once

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
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
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

#include <meta>
#include <vector>
#include <ranges>
#include <algorithm>

#include <sqlpp26/core/basic/column.h>
#include <sqlpp26/core/type_traits.h>
#include <sqlpp26/core/basic/fixed_string.h>
#include <sqlpp26/core/basic/column_spec.h>

namespace sqlpp {

template <typename TableSpec, fixed_string Alias>
struct table_as : public TableSpec::generator::template table_as_columns<Alias>::type {};

template <typename TableSpec, fixed_string Alias>
struct table_spec_of<table_as<TableSpec, Alias>>
{
  using type = TableSpec;
};

class enable_table_as {
 public:
  template <fixed_string Alias, typename Table>
  constexpr auto as(this Table self) {
    return table_as<table_spec_of_t<Table>, Alias>{};
  }
};

template <typename TableSpec>
struct table : public TableSpec::generator::columns, public enable_table_as {};

template <typename TableSpec>
struct table_spec_of<table<TableSpec>>
{
  using type = TableSpec;
};

// Table generator
template <typename TableSpec, fixed_string Name, typename... ColumnSpecs>
struct make_table {
  struct columns;
  consteval {
    std::vector<std::meta::info> column_data_members;
    template for (constexpr auto index : std::views::iota(size_t{}, sizeof...(ColumnSpecs))) {
      column_data_members.push_back(std::meta::data_member_spec(
          substitute(^^sqlpp::column, {^^table<TableSpec>, ^^index}),
          {.name = ColumnSpecs...[index]::name}));
    }
    define_aggregate(^^columns, column_data_members);
  }

  template <fixed_string Alias>
  struct table_as_columns {
    struct type;
    consteval {
      std::vector<std::meta::info> column_data_members;
      template for (constexpr auto index : std::views::iota(size_t{}, sizeof...(ColumnSpecs))) {
        column_data_members.push_back(std::meta::data_member_spec(
            substitute(^^sqlpp::column, {^^table_as<TableSpec, Alias>, ^^index}),
            {.name = ColumnSpecs...[index]::name}));
      }
      define_aggregate(^^type, column_data_members);
    }
  };

  template<size_t Idx>
  using column_spec = ColumnSpecs...[Idx];
};

}  // namespace sqlpp

#if 0
#include <sqlpp26/core/basic/all_of.h>
#include <sqlpp26/core/basic/column.h>
#include <sqlpp26/core/basic/enable_join.h>
#include <sqlpp26/core/basic/join.h>
#include <sqlpp26/core/basic/table_as.h>
#include <sqlpp26/core/detail/type_set.h>
#include <sqlpp26/core/type_traits.h>

#include <sqlpp26/core/name/create_reflection_name_tag.h>

namespace sqlpp {
template <typename TableSpec>
struct table_t : public TableSpec::template _table_columns<table_t<TableSpec>>,
                 public enable_join {
  template <typename NameTagProvider>
  constexpr auto as(const NameTagProvider& /*unused*/) const
      -> table_as_t<TableSpec, name_tag_of_t<NameTagProvider>> {
    return {};
  }

#if SQLPP_INCLUDE_REFLECTION == 1
  template <::sqlpp::detail::fixed_string Alias>
  constexpr auto as() const -> table_as_t<
      TableSpec,
      name_tag_of_t<decltype(::sqlpp::meta::make_alias<Alias>())>> {
    return {};
  }
#endif
};

template <typename TableSpec>
struct is_raw_table<table_t<TableSpec>> : public std::true_type {};

template <typename TableSpec>
struct is_table<table_t<TableSpec>> : public std::true_type {};

template <typename TableSpec>
struct name_tag_of<table_t<TableSpec>> : public name_tag_of<TableSpec> {};

template <typename TableSpec>
struct provided_tables_of<table_t<TableSpec>> {
  using type = sqlpp::detail::type_set<table_t<TableSpec>>;
};

template <typename TableSpec>
struct required_insert_columns_of<table_t<TableSpec>> {
  using type = typename TableSpec::_required_insert_columns;
};

template <typename Context, typename TableSpec>
auto to_sql_string(Context& context, const table_t<TableSpec>& /*unused*/)
    -> std::string {
  return name_to_sql_string(context, name_tag_of_t<TableSpec>{});
}
}  // namespace sqlpp
#endif
