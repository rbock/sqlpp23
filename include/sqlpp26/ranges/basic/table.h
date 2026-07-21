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

#include <meta>

#include <sqlpp26/core/basic/column_spec.h>
#include <sqlpp26/core/basic/table.h>


namespace sqlpp {

// Table generator
template <typename TableSpec, typename DataStruct>
struct make_stl_table {
  struct columns;
  static constexpr auto data_members = std::define_static_array(std::meta::nonstatic_data_members_of(^^DataStruct, std::meta::access_context::current()));
  consteval {
    std::vector<std::meta::info> column_data_members;
    template for (constexpr auto index : std::views::iota(size_t{}, data_members.size())) {
      column_data_members.push_back(std::meta::data_member_spec(
          substitute(^^sqlpp::column, {^^table<TableSpec>, ^^index}),
          {.name = std::meta::identifier_of(data_members[index])}));

    }
    define_aggregate(^^columns, column_data_members);
  }

  template <fixed_string Alias>
  struct table_as_columns {
    struct type;
    consteval {
      std::vector<std::meta::info> column_data_members;
      template for (constexpr auto index : std::views::iota(size_t{}, data_members.size())) {
        column_data_members.push_back(std::meta::data_member_spec(
            substitute(^^sqlpp::column, {^^table_as<TableSpec, Alias>, ^^index}),
            {.name = std::meta::identifier_of(data_members[index])}));
      }
      define_aggregate(^^type, column_data_members);
    }
  };

  template<size_t Idx>
  using column_type = [:std::meta::type_of(data_members[Idx]):];

  template<size_t Idx>
  static constexpr bool has_default = not (std::is_integral_v<column_type<Idx>> or std::is_floating_point_v<column_type<Idx>>);

  template<size_t Idx>
  using column_spec = column_spec<fixed_string<std::meta::identifier_of(data_members[Idx]).size()>(std::meta::identifier_of(data_members[Idx])), column_type<Idx>, has_default<Idx>>;

  using data_struct = DataStruct;
};

}  // namespace sqlpp


