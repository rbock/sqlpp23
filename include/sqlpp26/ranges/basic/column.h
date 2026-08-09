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

#include <sqlpp26/core/basic/column.h>
#include <sqlpp26/core/basic/table.h>

namespace sqlpp::ranges {

template <typename TableSpec, size_t Idx>
struct accessor {
  using data_struct = typename TableSpec::generator::data_struct;
  static constexpr auto data_members = std::define_static_array(std::meta::nonstatic_data_members_of(^^data_struct, std::meta::access_context::current()));

  constexpr const auto& operator()(const data_struct& t) const {
    return t.[:data_members[Idx]:];
  }

  constexpr auto& operator()(data_struct& t) const {
    return t.[:data_members[Idx]:];
  }
};

} // namespace sqlpp::ranges

namespace sqlpp {

template <typename TableSpec, std::size_t Idx>
struct name_of<ranges::accessor<TableSpec, Idx>> {
  static constexpr fixed_string value = name_of_v<column<table<TableSpec>, Idx>>;
};

template <typename TableSpec, size_t Idx>
constexpr auto to_filter_expression(const column<table<TableSpec>, Idx>) {
  return ranges::accessor<TableSpec, Idx>{};
}

}  // namespace sqlpp

