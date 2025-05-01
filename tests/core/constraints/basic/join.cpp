/*
 * Copyright (c) 2024, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
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

#include <sqlpp23/tests/core/constraints_helpers.h>

#include <sqlpp23/tests/core/tables.h>

namespace {
SQLPP_CREATE_NAME_TAG(something);

// Returns true if `JOIN(declval<Lhs>(), declval<Rhs>)` is a valid function
// call.
#define MAKE_CAN_CALL_JOIN_WITH(JOIN)                          \
  template <typename Lhs, typename Rhs, typename = void>       \
  struct can_call_##JOIN##_with : public std::false_type {};   \
                                                               \
  template <typename Lhs, typename Rhs>                        \
  struct can_call_##JOIN##_with<                               \
      Lhs, Rhs,                                                \
      std::void_t<decltype(sqlpp::JOIN(std::declval<Lhs>(),    \
                                       std::declval<Rhs>()))>> \
      : public std::true_type {};

MAKE_CAN_CALL_JOIN_WITH(join);
MAKE_CAN_CALL_JOIN_WITH(inner_join);
MAKE_CAN_CALL_JOIN_WITH(left_outer_join);
MAKE_CAN_CALL_JOIN_WITH(right_outer_join);
MAKE_CAN_CALL_JOIN_WITH(full_outer_join);
MAKE_CAN_CALL_JOIN_WITH(cross_join);

// Returns true if `JOIN(declval<Lhs>(), declval<Rhs>).on(Expr)` is a valid
// function call.
#define MAKE_CAN_CALL_JOIN_ON_WITH(JOIN)                                \
  template <typename Lhs, typename Rhs, typename Expr, typename = void> \
  struct can_call_##JOIN##_on_with : public std::false_type {};         \
                                                                        \
  template <typename Lhs, typename Rhs, typename Expr>                  \
  struct can_call_##JOIN##_on_with<                                     \
      Lhs, Rhs, Expr,                                                   \
      std::void_t<decltype(sqlpp::JOIN(std::declval<Lhs>(),             \
                                       std::declval<Rhs>())             \
                               .on(std::declval<Expr>()))>>             \
      : public std::true_type {};

MAKE_CAN_CALL_JOIN_ON_WITH(join);
MAKE_CAN_CALL_JOIN_ON_WITH(inner_join);
MAKE_CAN_CALL_JOIN_ON_WITH(left_outer_join);
MAKE_CAN_CALL_JOIN_ON_WITH(right_outer_join);
MAKE_CAN_CALL_JOIN_ON_WITH(full_outer_join);
MAKE_CAN_CALL_JOIN_ON_WITH(cross_join);

#define CAN_CALL_ALL_JOINS_WITH(LHS, RHS)                                      \
  static_assert(can_call_join_with<decltype(LHS), decltype(RHS)>::value, "");  \
  static_assert(can_call_inner_join_with<decltype(LHS), decltype(RHS)>::value, \
                "");                                                           \
  static_assert(                                                               \
      can_call_left_outer_join_with<decltype(LHS), decltype(RHS)>::value, ""); \
  static_assert(                                                               \
      can_call_right_outer_join_with<decltype(LHS), decltype(RHS)>::value,     \
      "");                                                                     \
  static_assert(                                                               \
      can_call_full_outer_join_with<decltype(LHS), decltype(RHS)>::value, ""); \
  static_assert(can_call_cross_join_with<decltype(LHS), decltype(RHS)>::value, \
                "");

#define CANNOT_CALL_ANY_JOIN_WITH(LHS, RHS)                                    \
  static_assert(not can_call_join_with<decltype(LHS), decltype(RHS)>::value,   \
                "");                                                           \
  static_assert(                                                               \
      not can_call_inner_join_with<decltype(LHS), decltype(RHS)>::value, "");  \
  static_assert(                                                               \
      not can_call_left_outer_join_with<decltype(LHS), decltype(RHS)>::value,  \
      "");                                                                     \
  static_assert(                                                               \
      not can_call_right_outer_join_with<decltype(LHS), decltype(RHS)>::value, \
      "");                                                                     \
  static_assert(                                                               \
      not can_call_full_outer_join_with<decltype(LHS), decltype(RHS)>::value,  \
      "");                                                                     \
  static_assert(                                                             \
      not can_call_cross_join_with<decltype(LHS), decltype(RHS)>::value, "");

#define CAN_CALL_ALL_JOINS_ON_WITH(LHS, RHS, EXPR)                             \
  static_assert(can_call_join_on_with<decltype(LHS), decltype(RHS),            \
                                      decltype(EXPR)>::value,                  \
                "");                                                           \
  static_assert(can_call_inner_join_on_with<decltype(LHS), decltype(RHS),      \
                                            decltype(EXPR)>::value,            \
                "");                                                           \
  static_assert(can_call_left_outer_join_on_with<decltype(LHS), decltype(RHS), \
                                                 decltype(EXPR)>::value,       \
                "");                                                           \
  static_assert(                                                               \
      can_call_right_outer_join_on_with<decltype(LHS), decltype(RHS),          \
                                        decltype(EXPR)>::value,                \
      "");                                                                     \
  static_assert(can_call_full_outer_join_on_with<decltype(LHS), decltype(RHS), \
                                                 decltype(EXPR)>::value,       \
                "");

#define CANNOT_CALL_ALL_JOINS_ON_WITH(LHS, RHS, EXPR)                         \
  CAN_CALL_ALL_JOINS_WITH(LHS, RHS);                                          \
  static_assert(not can_call_join_on_with<decltype(LHS), decltype(RHS),  \
                                               decltype(EXPR)>::value,        \
                     "");                                                     \
  static_assert(not can_call_inner_join_on_with<decltype(LHS), decltype(RHS), \
                                                decltype(EXPR)>::value,       \
                "");                                                          \
  static_assert(                                                              \
      not can_call_left_outer_join_on_with<decltype(LHS), decltype(RHS),      \
                                           decltype(EXPR)>::value,            \
      "");                                                                    \
  static_assert(                                                              \
      not can_call_right_outer_join_on_with<decltype(LHS), decltype(RHS),     \
                                            decltype(EXPR)>::value,           \
      "");                                                                    \
  static_assert(                                                              \
      not can_call_full_outer_join_on_with<decltype(LHS), decltype(RHS),      \
                                           decltype(EXPR)>::value,            \
      "");

struct weird_table : public sqlpp::enable_join {};

}  // namespace

namespace sqlpp {
template <>
struct is_table<weird_table> : public std::true_type {};
template <>
struct required_tables_of<weird_table> {
  using type = detail::type_vector<test::TabBar>;
};
}  // namespace sqlpp

int main() {
  const auto maybe = true;
  const auto foo = test::TabFoo{};
  const auto bar = test::TabBar{};
  const auto aFoo = foo.as(sqlpp::alias::a);
  const auto bFoo = foo.as(sqlpp::alias::b);

  // OK
  CAN_CALL_ALL_JOINS_WITH(bar, foo);
  CAN_CALL_ALL_JOINS_WITH(bar, foo.as(something));

  // Cannot join with a non-table
  CANNOT_CALL_ANY_JOIN_WITH(bar, foo.id);

  // Cannot join two identical tables.
  CANNOT_CALL_ANY_JOIN_WITH(foo, foo);

  // Cannot join two tables with identical names.
  CANNOT_CALL_ANY_JOIN_WITH(foo, bar.as(foo));

  // JOIN must not be called with tables that depend on other tables.
  // Not sure this can happen in the wild, which is why we are using the
  // `weird_table` to simulate the situation.
  CANNOT_CALL_ANY_JOIN_WITH(weird_table{}, foo);
  CANNOT_CALL_ANY_JOIN_WITH(foo, weird_table{});

  // JOIN ... ON can be called with any boolean expression, but will fail with
  // static assert if it uses the wrong tables. Here, bFoo is not provided by
  // the join.
  CANNOT_CALL_ALL_JOINS_ON_WITH(foo, bar, bFoo.id == bar.id);
  CANNOT_CALL_ALL_JOINS_ON_WITH(foo, bar, bFoo.id == aFoo.id);
  //
  // Here, bar is provided /dynamically/ by the first join, but required
  // /statically/ by the second join.
  CANNOT_CALL_ALL_JOINS_ON_WITH(foo.join(dynamic(maybe, bar))
                                  .on(foo.id == bar.id), aFoo, bar.id == aFoo.id);
}
