#pragma once

/*
 * Copyright (c) 2024, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sqlpp11/core/type_traits.h>
#include <sqlpp11/core/operator/enable_as.h>
#include <sqlpp11/core/basic/enable_join.h>

namespace sqlpp
{
  template<typename NameTagProvider>
  struct select_ref_t{};

   template<typename NameTagProvider>
    struct name_tag_of<select_ref_t<NameTagProvider>> : public name_tag_of<NameTagProvider>
    {};


  // select_member is a helper to add column data members to `select_as_t`.
  template <typename NameTagProvider, typename FieldSpec>
  struct select_member
  {
    using type = member_t<FieldSpec, column_t<select_ref_t<NameTagProvider>, FieldSpec>>;
  };

#warning: Need to document that you need to be a bit careful with aliased SELECTs as we use select_ref in columns.
  template <typename Select, typename NameTagProvider, typename... FieldSpecs>
  struct select_as_t : public select_member<NameTagProvider, FieldSpecs>::type...,
                       public enable_join<select_as_t<Select, NameTagProvider, FieldSpecs...>>
  {
    select_as_t(Select select) : _select(select)
    {
    }

    select_as_t(const select_as_t& rhs) = default;
    select_as_t(select_as_t&& rhs) = default;
    select_as_t& operator=(const select_as_t& rhs) = default;
    select_as_t& operator=(select_as_t&& rhs) = default;
    ~select_as_t() = default;

    using _column_tuple_t = std::tuple<column_t<select_ref_t<NameTagProvider>, FieldSpecs>...>;

    Select _select;
  };

  // The Select expression has a value in case it has just one column selected.
  template<typename Select, typename NameTagProvider, typename ColumnSpec>
    struct value_type_of<select_as_t<Select, NameTagProvider, ColumnSpec>> : value_type_of<Select> {};

  template<typename Select, typename NameTagProvider, typename... FieldSpecs>
    struct name_tag_of<select_as_t<Select, NameTagProvider, FieldSpecs...>> : name_tag_of<NameTagProvider> {};

  template <typename Select, typename NameTagProvider, typename... FieldSpecs>
  struct is_table<select_as_t<Select, NameTagProvider, FieldSpecs...>>
      : std::integral_constant<bool, Select::_can_be_used_as_table()>
  {
  };

  template <typename Select, typename NameTagProvider, typename... FieldSpecs>
  struct provided_tables_of<select_as_t<Select, NameTagProvider, FieldSpecs...>>
      : public std::conditional<Select::_can_be_used_as_table(),
                                sqlpp::detail::type_vector<select_as_t<Select, NameTagProvider, FieldSpecs...>>,
                                sqlpp::detail::type_vector<>>
  {
  };

  template <typename Select, typename NameTagProvider, typename... FieldSpecs>
  struct provided_static_tables_of<select_as_t<Select, NameTagProvider, FieldSpecs...>>
      : public provided_tables_of<select_as_t<Select, NameTagProvider, FieldSpecs...>>
  {
  };

  template <typename Select, typename NameTagProvider, typename... FieldSpecs>
  struct provided_optional_tables_of<select_as_t<Select, NameTagProvider, FieldSpecs...>>
      : public provided_tables_of<select_as_t<Select, NameTagProvider, FieldSpecs...>>
  {
  };

#warning: V1.0 has empty nodes. Is that correct? In either case document the decision here.
#warning: Need to add required tables of

  template <typename Context, typename Select, typename NameTagProvider, typename... FieldSpecs>
  auto to_sql_string(Context& context, const select_as_t<Select, NameTagProvider, FieldSpecs...>& t) -> std::string
  {
    return operand_to_sql_string(context, t._select) + " AS " +name_to_sql_string(context, name_tag_of_t<NameTagProvider>::name);
  }
}  // namespace sqlpp
