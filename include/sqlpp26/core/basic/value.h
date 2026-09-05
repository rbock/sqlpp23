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

#include <sqlpp26/core/operator/enable_as.h>
#include <sqlpp26/core/operator/enable_comparison.h>
#include <sqlpp26/core/type_traits.h>

namespace sqlpp {
template <typename T>
struct value_t : public enable_as, public enable_comparison {
  constexpr value_t(T t) : _value(std::move(t)) {}
  value_t(const value_t&) = default;
  value_t(value_t&&) = default;
  value_t& operator=(const value_t&) = default;
  value_t& operator=(value_t&&) = default;
  ~value_t() = default;

  T _value;
};

template <typename T>
struct data_type_of<value_t<T>> {
  using type = data_type_of_t<T>;
};

template <typename Select>
requires(is_statement_v<Select>)
struct data_type_of<value_t<Select>> {
  using type = statement_data_type_of_t<Select>;
};

template <typename T>
struct nodes_of<value_t<T>> {
  // Required in case of value(select(...)).
  using type = detail::type_vector<T>;
};

template <typename T>
struct requires_parentheses<value_t<T>> : public requires_parentheses<T> {};

template <typename Context, typename T>
auto to_sql_string(Context& context, const value_t<T>& t) -> std::string {
  return to_sql_string(context, t._value);
}

template <typename T>
  requires(is_data_type_v<T>)
constexpr auto value(T t) -> value_t<T> {
  return {std::move(t)};
}

template <typename Select>
  requires(has_statement_data_type_v<Select> and is_statement_v<Select>)
constexpr auto value(Select t) -> value_t<Select> {
  Select::check_basic_consistency();
  return {std::move(t)};
}

}  // namespace sqlpp
