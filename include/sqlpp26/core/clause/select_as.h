#pragma once

/*
 * Copyright (c) 2024, Roland Bock
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

#include <sqlpp26/core/basic/enable_join.h>
#include <sqlpp26/core/basic/field_column.h>
#include <sqlpp26/core/query/statement.h>
#include <sqlpp26/core/reader.h>
#include <sqlpp26/core/query/result_row_fwd.h>
#include <sqlpp26/core/type_traits.h>

namespace sqlpp {
template <fixed_string Name>
struct select_ref_t {};

template <fixed_string Name>
struct name_of<select_ref_t<Name>> {
  static constexpr std::string_view value = Name;
};

template <typename Select, fixed_string Name, typename ResultRow>
struct select_as_generator;

template <typename Select, fixed_string Name, typename... FieldSpecs>
struct select_as_generator<Select, Name, result_row_t<FieldSpecs...>>{
  struct columns;
  consteval {
    std::vector<std::meta::info> column_data_members;
    std::array field_specs = {^^FieldSpecs...};
    template for (constexpr auto index : std::views::iota(size_t{}, sizeof...(FieldSpecs))) {
      using FieldSpec = FieldSpecs...[index];
      column_data_members.push_back(std::meta::data_member_spec(
          substitute(^^sqlpp::field_column, {^^select_ref_t<Name>, field_specs[index]}),
          {.name = FieldSpec::name}));
    }
    define_aggregate(^^columns, column_data_members);
  }
};

template <typename Select, fixed_string Name>
struct select_as
    : public select_as_generator<Select, Name, get_result_row_t<Select>>::columns,
      public enable_join {
  constexpr select_as(Select select) : _expression(std::move(select)) {}

  select_as(const select_as& rhs) = default;
  select_as(select_as&& rhs) = default;
  select_as& operator=(const select_as& rhs) = default;
  select_as& operator=(select_as&& rhs) = default;
  ~select_as() = default;

 private:
  friend reader_t;
  Select _expression;
};

template <typename Context,
          typename Select,
          fixed_string Name,
          typename... FieldSpecs>
auto to_sql_string(Context& context,
                   const select_as<Select, Name>& t)
    -> std::string {
  return operand_to_sql_string(context, read.expression(t)) + " AS " +
         name_to_sql_string(context, Name);
}

// No data_type_of defined. select_as represents a table, not a value.
// Rationale: select.as() requires prepare_check to be used as tbale, whereas
// using as value just requires consistency.

template <typename Select, fixed_string Name>
struct name_of<select_as<Select, Name>> {
  static constexpr std::string_view value = Name;
};

// We need to track nodes to find parameters or required tables in sub selects.
template <typename Select, fixed_string Name>
struct nodes_of<select_as<Select, Name>> {
  using type = detail::type_vector<Select>;
};

// TODO Why isn't this simply true? We constructed a select_as. We could not do that unless it could be used as table...
template <typename Select, fixed_string Name>
struct is_table<select_as<Select, Name>>
    : public can_be_used_as_table<Select> {};

#if 0
template <typename Select, fixed_string Name>
struct provided_tables_of<select_as<Select, Name>>
    : public std::conditional<can_be_used_as_table<Select>::value,
                              sqlpp::detail::type_set<select_ref_t<Name>>,
                              sqlpp::detail::type_set<>> {};

#endif
}  // namespace sqlpp
