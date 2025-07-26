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

#include <sqlpp23/core/logic.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
template <typename L, typename R1, typename R2>
struct between_expression;

template <typename L, typename Operator, typename R>
struct comparison_expression;

template <typename L, typename Operator, typename Container>
struct in_expression;

template <typename L>
struct sort_order_expression;

struct op_is_null {
  static constexpr auto symbol = " IS ";
};

template <typename L>
constexpr auto is_null(L l)
    -> comparison_expression<L, op_is_null, std::nullopt_t> {
  return {l, std::nullopt};
}

struct op_is_not_null {
  static constexpr auto symbol = " IS NOT ";
};

template <typename L>
constexpr auto is_not_null(L l)
    -> comparison_expression<L, op_is_not_null, std::nullopt_t> {
  return {l, std::nullopt};
}

struct op_is_distinct_from {
  static constexpr auto symbol = " IS DISTINCT FROM ";  // sql standard
  // mysql has NULL-safe equal `<=>` which is_null equivalent to `IS NOT
  // DISTINCT FROM` sqlite3 has `IS NOT`
};

template <typename L, typename R>
  requires(values_are_comparable<L, R>::value)
constexpr auto is_distinct_from(L l, R r)
    -> comparison_expression<L, op_is_distinct_from, R> {
  return {l, r};
}

struct op_is_not_distinct_from {
  static constexpr auto symbol = " IS NOT DISTINCT FROM ";  // sql standard
  // mysql has NULL-safe equal `<=>`
  // sqlite3 has `IS`
};

template <typename L, typename R>
  requires(values_are_comparable<L, R>::value)
constexpr auto is_not_distinct_from(L l, R r)
    -> comparison_expression<L, op_is_not_distinct_from, R> {
  return {l, r};
}

struct op_like {
  static constexpr auto symbol = " LIKE ";
};

template <typename L, typename R>
  requires(is_text<L>::value and is_text<R>::value)
constexpr auto like(L l, R r) -> comparison_expression<L, op_like, R> {
  return {std::move(l), std::move(r)};
}

struct operator_in {
  static constexpr auto symbol = " IN";
};

struct operator_not_in {
  static constexpr auto symbol = " NOT IN";
};

template <typename L, typename... Args>
  requires((sizeof...(Args) != 0) and
           logic::all<values_are_comparable<L, Args>::value...>::value)
constexpr auto in(L lhs, std::tuple<Args...> args)
    -> in_expression<L, operator_in, std::tuple<Args...>> {
  static_assert(sizeof...(Args) > 0, "");
  return {std::move(lhs), std::move(args)};
}

template <typename L, typename... Args>
  requires((sizeof...(Args) != 0) and
           logic::all<values_are_comparable<L, Args>::value...>::value)
constexpr auto in(L lhs, Args... args)
    -> in_expression<L, operator_in, std::tuple<Args...>> {
  static_assert(sizeof...(Args) > 0, "");
  return {std::move(lhs), std::make_tuple(std::move(args)...)};
}

template <typename L, typename Arg>
  requires(values_are_comparable<L, Arg>::value)
constexpr auto in(L lhs, std::vector<Arg> args)
    -> in_expression<L, operator_in, std::vector<Arg>> {
  return {std::move(lhs), std::move(args)};
}

template <typename L, typename... Args>
  requires((sizeof...(Args) != 0) and
           logic::all<values_are_comparable<L, Args>::value...>::value)
constexpr auto not_in(L lhs, std::tuple<Args...> args)
    -> in_expression<L, operator_not_in, std::tuple<Args...>> {
  return {std::move(lhs), std::move(args)};
}

template <typename L, typename... Args>
  requires((sizeof...(Args) != 0) and
           logic::all<values_are_comparable<L, Args>::value...>::value)
constexpr auto not_in(L lhs, Args... args)
    -> in_expression<L, operator_not_in, std::tuple<Args...>> {
  return {std::move(lhs), std::make_tuple(std::move(args)...)};
}

template <typename L, typename Arg>
  requires(values_are_comparable<L, Arg>::value)
constexpr auto not_in(L lhs, std::vector<Arg> args)
    -> in_expression<L, operator_not_in, std::vector<Arg>> {
  return {std::move(lhs), std::move(args)};
}

template <typename L, typename R1, typename R2>
  requires(values_are_comparable<L, R1>::value and
           values_are_comparable<L, R2>::value)
constexpr auto between(L l, R1 r1, R2 r2) -> between_expression<L, R1, R2> {
  return {std::move(l), std::move(r1), std::move(r2)};
}

enum class sort_type {
  asc,
  desc,
};

template <typename L>
requires(values_are_comparable<L, L>::value)
constexpr auto asc(L l) -> sort_order_expression<L> {
  return {l, sort_type::asc};
}

template <typename L>
requires(values_are_comparable<L, L>::value)
constexpr auto desc(L l) -> sort_order_expression<L> {
  return {l, sort_type::desc};
}

template <typename L>
requires(values_are_comparable<L, L>::value)
constexpr auto order(L l, sort_type order) -> sort_order_expression<L> {
  return {l, order};
}

}  // namespace sqlpp
