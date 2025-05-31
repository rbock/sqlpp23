#pragma once

/*
Copyright (c) 2025, Roland Bock
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <utility>

#include <sqlpp23/core/default_value.h>
#include <sqlpp23/core/operator/enable_as.h>
#include <sqlpp23/core/operator/enable_comparison.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
template <typename Expression, typename DataType>
struct cast_t  : public enable_as, public enable_comparison {
  constexpr cast_t(Expression expression) : _expression(std::move(expression)) {}
  cast_t(const cast_t&) = default;
  cast_t(cast_t&&) = default;
  cast_t& operator=(const cast_t&) = default;
  cast_t& operator=(cast_t&&) = default;
  ~cast_t() = default;

  Expression _expression;
};

template <typename Expression, typename DataType>
struct data_type_of<cast_t<Expression, DataType>> {
  using type = std::optional<DataType>;
};

template <typename Expression, typename DataType>
struct nodes_of<cast_t<Expression, DataType>> {
  using type = detail::type_vector<Expression, DataType>;
};

template <typename Context, typename Expression, typename DataType>
auto to_sql_string(Context& context, const cast_t<Expression, DataType>& t)
    -> std::string {
  return std::format(
      "CAST({} AS {})",
      operand_to_sql_string(context, t._expression),
      data_type_to_sql_string(context, DataType{}));
}

template <typename DataType>
struct cast_as_t{
};

template <typename DataType>
struct data_type_of<cast_as_t<DataType>> {
  using type = DataType;
};

template <typename DataType>
  requires(is_data_type<DataType>::value)
constexpr auto as(DataType)
    -> cast_as_t<DataType> {
  return {};
}

template <typename Expression, typename DataType>
  requires(has_data_type_v<Expression> and
           (is_text<Expression>::value or is_text<cast_as_t<DataType>>::value or
            values_are_comparable<Expression, cast_as_t<DataType>>::value))
constexpr auto cast(Expression expression, cast_as_t<DataType>)
    -> cast_t<Expression, DataType> {
  return {std::move(expression)};
}

template <typename DataType>
constexpr auto cast(std::nullopt_t, cast_as_t<DataType>)
    -> cast_t<std::nullopt_t, DataType> {
  return {std::nullopt};
}

}  // namespace sqlpp
