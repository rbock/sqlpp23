#pragma once

/*
Copyright (c) 2018, Roland Bock
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

#include <sqlpp23/core/noop.h>
#include <sqlpp23/core/operator/any.h>
#include <sqlpp23/core/operator/enable_as.h>
#include <sqlpp23/core/operator/enable_comparison.h>
#include <sqlpp23/core/reader.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
template <typename Lhs, typename Operator, typename Rhs>
struct comparison_expression
    : public enable_as, enable_comparison{
  constexpr comparison_expression(Lhs l, Rhs r)
      : _lhs(std::move(l)), _rhs(std::move(r)) {}
  comparison_expression(const comparison_expression&) = default;
  comparison_expression(comparison_expression&&) = default;
  comparison_expression& operator=(const comparison_expression&) = default;
  comparison_expression& operator=(comparison_expression&&) = default;
  ~comparison_expression() = default;

 private:
  friend reader_t;
  Lhs _lhs;
  Rhs _rhs;
};

template <typename Lhs, typename Operator, typename Rhs>
struct data_type_of<comparison_expression<Lhs, Operator, Rhs>>
    : std::conditional<
          sqlpp::is_optional<data_type_of_t<Lhs>>::value or
              sqlpp::is_optional<data_type_of_t<remove_any_t<Rhs>>>::value,
          std::optional<boolean>,
          boolean> {};

template <typename Lhs>
struct data_type_of<comparison_expression<Lhs, op_is_null, std::nullopt_t>> {
  using type = boolean;
};

template <typename Lhs>
struct data_type_of<
    comparison_expression<Lhs, op_is_not_null, std::nullopt_t>> {
  using type = boolean;
};

template <typename Lhs, typename Rhs>
struct data_type_of<comparison_expression<Lhs, op_is_distinct_from, Rhs>> {
  using type = boolean;
};

template <typename Lhs, typename Rhs>
struct data_type_of<comparison_expression<Lhs, op_is_not_distinct_from, Rhs>> {
  using type = boolean;
};

template <typename Lhs, typename Operator, typename Rhs>
struct nodes_of<comparison_expression<Lhs, Operator, Rhs>> {
  using type = detail::type_vector<Lhs, Rhs>;
};

template <typename Lhs, typename Operator, typename Rhs>
struct requires_parentheses<comparison_expression<Lhs, Operator, Rhs>>
    : public std::true_type {};

template <typename Context, typename Lhs, typename Operator, typename Rhs>
auto to_sql_string(Context& context,
                   const comparison_expression<Lhs, Operator, Rhs>& t)
    -> std::string {
  // Note: Temporary required to enforce parameter ordering.
  auto ret_val = operand_to_sql_string(context, read.lhs(t)) + Operator::symbol;
  return ret_val + operand_to_sql_string(context, read.rhs(t));
}

struct less {
  static constexpr auto symbol = " < ";
};

// We are using remove_any_t in the basic comparison operators to allow
// comparison with ANY-expressions. Note: any_t does not have a specialization
// for data_type_of to disallow it from being used in other contexts.
template <typename Lhs, typename Rhs>
  requires(values_are_comparable<Lhs, remove_any_t<Rhs>>::value)
constexpr auto operator<(Lhs l, Rhs r)
    -> comparison_expression<Lhs, less, Rhs> {
  return {std::move(l), std::move(r)};
}

struct less_equal {
  static constexpr auto symbol = " <= ";
};

template <typename Lhs, typename Rhs>
  requires(values_are_comparable<Lhs, remove_any_t<Rhs>>::value)
constexpr auto operator<=(Lhs l, Rhs r)
    -> comparison_expression<Lhs, less_equal, Rhs> {
  return {std::move(l), std::move(r)};
}

struct equal_to {
  static constexpr auto symbol = " = ";
};

template <typename Lhs, typename Rhs>
  requires(values_are_comparable<Lhs, remove_any_t<Rhs>>::value)
constexpr auto operator==(Lhs l, Rhs r)
    -> comparison_expression<Lhs, equal_to, Rhs> {
  return {std::move(l), std::move(r)};
}

struct not_equal_to {
  static constexpr auto symbol = " <> ";
};

template <typename Lhs, typename Rhs>
  requires(values_are_comparable<Lhs, remove_any_t<Rhs>>::value)
constexpr auto operator!=(Lhs l, Rhs r)
    -> comparison_expression<Lhs, not_equal_to, Rhs> {
  return {std::move(l), std::move(r)};
}

struct greater_equal {
  static constexpr auto symbol = " >= ";
};

template <typename Lhs, typename Rhs>
  requires(values_are_comparable<Lhs, remove_any_t<Rhs>>::value)
constexpr auto operator>=(Lhs l, Rhs r)
    -> comparison_expression<Lhs, greater_equal, Rhs> {
  return {std::move(l), std::move(r)};
}

struct greater {
  static constexpr auto symbol = " > ";
};

template <typename Lhs, typename Rhs>
  requires(values_are_comparable<Lhs, remove_any_t<Rhs>>::value)
constexpr auto operator>(Lhs l, Rhs r)
    -> comparison_expression<Lhs, greater, Rhs> {
  return {std::move(l), std::move(r)};
}

}  // namespace sqlpp
