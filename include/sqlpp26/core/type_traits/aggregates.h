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

#include <sqlpp26/core/detail/type_vector.h>
#include <sqlpp26/core/logic.h>
#include <sqlpp26/core/query/dynamic_fwd.h>
#include <sqlpp26/core/type_traits/nodes_of.h>
#include "sqlpp26/core/detail/type_info_set.h"

namespace sqlpp {
// We don't want to mix aggregate and non-aggregate expressions as the results
// are unspecified. Aggregates are either results of aggregate functions or
// GROUP BY columns. Non-aggregates are columns (unless they are aggregate
// columns). Constant values are neutral.

template <typename T>
struct is_aggregate_function : public std::false_type {};

template <typename T>
inline constexpr bool is_aggregate_function_v = is_aggregate_function<T>::value;

// Finds calls to aggregate functions (avg, count, max, min, sum) in
// expressions. This is important as aggregated functions must not be nested.
template <typename T>
struct contains_aggregate_function
    : public std::integral_constant<
          bool,
          is_aggregate_function<T>::value or
              contains_aggregate_function<nodes_of_t<T>>::value> {};

template <typename... T>
struct contains_aggregate_function<detail::type_vector<T...>>
    : public std::integral_constant<
          bool,
          logic::any<(is_aggregate_function<T>::value or
                      contains_aggregate_function<T>::value)...>::value> {};

// Obtain known aggregate columns, i.e. GROUP BY columns.
template <typename T>
struct known_aggregate_columns_of {
  static consteval detail::type_info_set func() { return {}; }
};

template <typename T>
struct known_static_aggregate_columns_of {
  static consteval detail::type_info_set func() { return {}; }
};

template <typename T>
struct is_aggregate_neutral : public std::true_type {};

// Checks if T is an aggregate expression, i.e. either
//  - T is an aggregate function,
//  - T is a known aggregate, or
//  - T is aggregate-neutral, or
//  - T exclusively exists of aggregate expressions.
// @KnownAggregateColumns: type_set as obtained through
// known_aggregate_columns_of
template <typename Statement, typename... T>
consteval auto is_aggregate_expression(const detail::type_vector<T...>&) -> bool;

template <typename Statement, typename T>
consteval auto is_aggregate_expression() -> bool {
  if (is_aggregate_function_v<T>) {
    return true;
  } else if (std::ranges::contains(Statement::get_known_aggregate_columns_of(), ^^T)) {
    return true;
  } else if (not nodes_of_t<T>::empty()) {
    return is_aggregate_expression<Statement>(nodes_of_t<T>{});
  }
  return is_aggregate_neutral<T>::value;
}

template <typename Statement, typename... T>
consteval auto is_aggregate_expression(const detail::type_vector<T...>&) -> bool {
  return logic::all<is_aggregate_expression<Statement, T>()...>::value;
};

// Checks if the static part of T is an aggregate expression, see above.
// @KnownStaticAggregateColumns: type_set as obtained through
// known_static_aggregate_columns_of_t
template <typename Statement, typename... T>
consteval auto static_part_is_aggregate_expression(const detail::type_vector<T...>&) -> bool;

template <typename Statement, typename T>
consteval auto static_part_is_aggregate_expression() -> bool {
  if (is_dynamic_v<T>) {
    return true;
  } else if (is_aggregate_function_v<T>) {
    return true;
  } else if (std::ranges::contains(Statement::get_known_static_aggregate_columns_of(), ^^T)) {
    return true;
  } else if (not nodes_of_t<T>::empty()) {
    return static_part_is_aggregate_expression<Statement>(nodes_of_t<T>{});
  }
  return is_aggregate_neutral<T>::value;
}

template <typename Statement, typename... T>
consteval auto static_part_is_aggregate_expression(const detail::type_vector<T...>&) -> bool {
  return logic::all<static_part_is_aggregate_expression<Statement, T>()...>::value;
};

// Checks if T is an non-aggregate expression, i.e.
//  - T is not an aggregate function, and
//  - T is not a known aggregate, and
//  - T exclusively exists of non-aggregate expressions, or
//  - T is aggregate-neutral
// @KnownAggregateColumns: type_set as obtained through
// known_aggregate_columns_of
template <typename Statement, typename... T>
consteval auto is_non_aggregate_expression(const detail::type_vector<T...>&) -> bool;

template <typename Statement, typename T>
consteval auto is_non_aggregate_expression() -> bool {
if (is_aggregate_function_v<T>) {
    return false;
  } else if (std::ranges::contains(Statement::get_known_aggregate_columns_of(), ^^T)) {
    return false;
  } else if (is_non_aggregate_expression<Statement>(nodes_of_t<T>{})) {
    return true;
  }
  return nodes_of_t<T>::empty() and is_aggregate_neutral<T>::value;
}

template <typename Statement, typename... T>
consteval auto is_non_aggregate_expression(const detail::type_vector<T...>&) -> bool {
  return logic::all<is_non_aggregate_expression<Statement, T>()...>::value;
};

}  // namespace sqlpp
