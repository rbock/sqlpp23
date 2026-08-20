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

#include <sqlpp26/core/detail/flat_tuple.h>
#include <sqlpp26/core/operator/as_expression.h>
#include <sqlpp26/core/query/dynamic.h>
#include <sqlpp26/core/type_traits.h>

// Select columns require a value type and a name.
// They can be dynamic values and they can be as_expressions.
// These type traits consider `dynamic_t` and `as_expression`

namespace sqlpp {
// Get value type
template <typename T>
struct select_column_data_type_of : public data_type_of<remove_as_t<remove_dynamic_t<T>>> {};
template <typename T>
using select_column_data_type_of_t =
    typename select_column_data_type_of<T>::type;

template <typename T>
struct select_column_name_of : public name_of<remove_dynamic_t<T>> {};

template <typename T>
inline constexpr fixed_string select_column_name_of_v = select_column_name_of<T>::value;

template <typename T>
struct select_column_sql_name_of : public sql_name_of<remove_dynamic_t<T>> {};

template <typename T>
inline constexpr fixed_string select_column_sql_name_of_v = select_column_sql_name_of<T>::value;

// Test for value
template <typename T>
struct select_column_has_data_type
    : public has_data_type<remove_as_t<remove_dynamic_t<T>>> {};

// Test for name
template <typename T>
struct select_column_has_name : public has_name<remove_dynamic_t<T>> {};

template <typename... Columns>
struct select_columns_have_values {
  static constexpr bool value = select_columns_have_values<
      detail::flat_tuple_t<is_select_column, Columns...>>::value;
};

template <typename... Columns>
struct select_columns_have_values<std::tuple<Columns...>> {
  static constexpr bool value =
      logic::all<select_column_has_data_type<Columns>::value...>::value;
};

template <typename... Columns>
struct select_columns_have_names {
  static constexpr bool value = select_columns_have_names<
      detail::flat_tuple_t<is_select_column, Columns...>>::value;
};

template <typename... Columns>
struct select_columns_have_names<std::tuple<Columns...>> {
  static constexpr bool value =
      logic::all<select_column_has_name<Columns>::value...>::value;
};
}  // namespace sqlpp
