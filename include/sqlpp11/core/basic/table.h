#pragma once

/*
 * Copyright (c) 2013-2015, Roland Bock
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

#include <sqlpp11/core/basic/enable_join.h>
#include <sqlpp11/core/type_traits.h>
#include <sqlpp11/core/basic/table_alias.h>
#include <sqlpp11/core/basic/all_of.h>
#include <sqlpp11/core/basic/column.h>
#include <sqlpp11/core/detail/type_set.h>
#include <sqlpp11/core/basic/join.h>

namespace sqlpp
{
  template <typename TableSpec>
  struct table_t : public TableSpec::_table_columns<TableSpec>, public enable_join<table_t<TableSpec>>
  {
    using _traits = make_traits<no_value_t, tag::is_raw_table>;

    using _nodes = detail::type_vector<>;
    using _provided_tables = detail::type_set<TableSpec>;

    using _required_insert_columns = typename TableSpec::_required_insert_columns;
#warning: Need to inherit?
    //using _column_tuple_t = std::tuple<column_t<Table, ColumnSpec>...>;
    template <typename AliasProvider, typename T>
    using _foreign_table_alias_t = table_alias_t<AliasProvider, T>;
    template <typename AliasProvider>
    using _alias_t = table_alias_t<AliasProvider, TableSpec>;

    template <typename AliasProvider>
    _alias_t<AliasProvider> as(const AliasProvider& /*unused*/) const
    {
      return {};
    }

  };

  template <typename TableSpec>
  struct name_tag_of<table_t<TableSpec>>: public name_tag_of<TableSpec> {};

  template <typename TableSpec>
  struct is_table<table_t<TableSpec>>: public std::true_type {};

  template <typename Context, typename TableSpec>
  Context& serialize(Context& context, const table_t<TableSpec>& /*unused*/)
  {
    context << name_tag_of_t<TableSpec>::_name_t::template char_ptr<Context>();
    return context;
  }
}  // namespace sqlpp
