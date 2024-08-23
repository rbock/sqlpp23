#pragma once

/*
 * Copyright (c) 2013-2015, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sqlpp11/core/type_traits.h>
#include <sqlpp11/core/unconditional.h>
#include <sqlpp11/core/logic.h>

namespace sqlpp
{
  SQLPP_PORTABLE_STATIC_ASSERT(assert_on_is_expression_t, "argument is not an expression in on()");
  SQLPP_PORTABLE_STATIC_ASSERT(assert_on_is_boolean_expression_t, "argument is not a boolean expression in on()");

  template <typename Expr>
  struct check_on
  {
    using type = static_combined_check_t<static_check_t<is_boolean<Expr>::value, assert_on_is_boolean_expression_t>>;
  };

  template <typename Expr>
  using check_on_t = typename check_on<Expr>::type;

  template <typename Expression>
  struct on_t
  {
    using _traits = make_traits<no_value_t, tag::is_on>;
    using _nodes = detail::type_vector<Expression>;

    Expression _expression;
  };

  template <>
  struct on_t<unconditional_t>
  {
    using _traits = make_traits<no_value_t, tag::is_on>;
    using _nodes = detail::type_vector<>;
  };

  template<typename Expression>
    struct nodes_of<on_t<Expression>>
    {
      using type = sqlpp::detail::type_vector<Expression>;
    };

  template <typename Context>
  auto to_sql_string(Context& , const on_t<unconditional_t>&) -> std::string
  {
    return {};
  }

  template <typename Context, typename Expression>
  auto to_sql_string(Context& context, const on_t<Expression>& t) -> std::string
  {
    return  " ON " + to_sql_string(context, t._expression);
  }
}  // namespace sqlpp
