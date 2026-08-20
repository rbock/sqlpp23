#pragma once

/*
 * Copyright (c) 2013-2016, Roland Bock
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
#include <sqlpp26/core/basic/join.h>
#include <sqlpp26/core/to_sql_string.h>
#include <sqlpp26/core/type_traits.h>

namespace sqlpp {
template <typename TableSpec>
struct table;

template <typename TableSpec, fixed_string Alias>
struct table_as
    : public TableSpec::generator::template table_as_columns<Alias>::type,
      public enable_join {};

template <typename TableSpec, fixed_string Alias>
struct is_table<table_as<TableSpec, Alias>> : public std::true_type {};

template <typename TableSpec, fixed_string Alias>
struct name_of<table_as<TableSpec, Alias>> {
  static constexpr fixed_string value = Alias.data;
};

template <typename TableSpec, fixed_string Alias>
struct provided_tables_of<table_as<TableSpec, Alias>> {
  static consteval detail::type_info_set func() { 
    return detail::make_type_info_set<table_as<TableSpec, Alias>>();
  }
};

template <typename TableSpec, fixed_string Alias>
constexpr auto all_of(const table_as<TableSpec, Alias>& t) {
  const auto& [...columns] = t;

  return std::make_tuple(columns...);
}

template <typename Context, typename TableSpec, fixed_string Alias>
auto to_sql_string(Context& context, const table_as<TableSpec, Alias>&)
    -> std::string {
  return name_to_sql_string(context, name_of_v<table<TableSpec>>) + " AS " +
         name_to_sql_string(context, Alias);
}
}  // namespace sqlpp
