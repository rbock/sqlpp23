#pragma once

/*
 * Copyright (c) 2013-2015, Roland Bock
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

#include <type_traits>

#include <sqlpp26/core/clause/select_column_traits.h>
#include <sqlpp26/core/type_traits.h>

namespace sqlpp {
template <fixed_string Name, typename DataType>
struct field_spec {
  static constexpr fixed_string name = Name;
  using data_type = DataType;
};

template <typename Left, typename Right>
struct is_field_compatible {
  static constexpr auto value = false;
};

template <fixed_string LeftName,
          typename LeftDataType,
          fixed_string RightName,
          typename RightDataType>
struct is_field_compatible<field_spec<LeftName, LeftDataType>,
                           field_spec<RightName, RightDataType>> {
  static constexpr auto value =
      std::string_view(LeftName.data) == std::string_view(RightName.data) and
      std::is_same<remove_optional_t<result_data_type_of_t<LeftDataType>>,
                   remove_optional_t<result_data_type_of_t<RightDataType>>>::value and  // Same result data type
      (is_optional<LeftDataType>::value or
       !is_optional<RightDataType>::value);  // The left hand side determines
                                             // the result row and therefore
                                             // must allow NULL if the right
                                             // hand side allows it
};

// Tables can joined dynamically or as the optional part of an outer join. In
// that case, their respective columns can be NULL.
template <typename Statement, typename SelectColumn>
struct field_depends_on_optional_table {
  static constexpr bool _depends_on_optional_table =
      detail::make_joined_type_info_set(
          provided_optional_tables_of<Statement>::func(),
          required_tables_of<SelectColumn>::func())
          .size() < provided_optional_tables_of<Statement>::func().size() +
                        required_tables_of<SelectColumn>::func().size();
};

template <typename Statement, typename SelectColumn>
struct field_data_type {
  using type = select_column_data_type_of_t<SelectColumn>;
};
template <typename Statement, typename SelectColumn>
using field_data_type_t = typename field_data_type<Statement, SelectColumn>::type;

template <typename Statement, typename SelectColumn>
requires(field_depends_on_optional_table<Statement, SelectColumn>::value)
struct field_data_type<Statement, SelectColumn> {
  using type =
      sqlpp::force_optional_t<select_column_data_type_of_t<SelectColumn>>;
};



template <typename Statement, typename SelectColumn>
struct make_field_spec {
  using type =
      field_spec<select_column_name_of_v<SelectColumn>,
                   field_data_type_t<Statement, SelectColumn>>;
};

template <typename Statement, typename SelectColumn>
using make_field_spec_t = typename make_field_spec<Statement, SelectColumn>::type;

}  // namespace sqlpp
