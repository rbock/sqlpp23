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
#include <optional>

#include <sqlpp26/core/basic/fixed_string.h>

namespace sqlpp {

template <typename T, std::meta::info Templ>
consteval bool is_specialization_of() {
  constexpr auto type = std::meta::dealias(^^T);
  return std::meta::has_template_arguments(type) &&
         std::meta::template_of(type) == Templ;
}

template <typename Column>
struct column_spec_of;

template <typename Column>
using column_spec_of_t = typename column_spec_of<Column>::type;

template <fixed_string SqlName, typename DataType, bool HasDefault = false, fixed_string CppName = SqlName>
struct column_spec {
  static constexpr fixed_string name = CppName;
  static constexpr fixed_string sql_name = SqlName;
  using data_type = DataType;
  static constexpr bool has_default = HasDefault or is_specialization_of<DataType, ^^std::optional>();

  using with_default = column_spec<SqlName, DataType, true, CppName>;
  template<fixed_string NewCppName>
  using with_cpp_name = column_spec<SqlName, DataType, HasDefault, NewCppName>;
};
}  // namespace sqlpp
