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

#include <sqlpp23/core/clause/expression_static_check.h>
#include <sqlpp23/core/detail/type_set.h>
#include <sqlpp23/core/query/statement.h>
#include <sqlpp23/core/reader.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
template <typename Expression>
struct offset_t {
  offset_t(Expression expression) : _expression(expression) {}
  offset_t(const offset_t&) = default;
  offset_t(offset_t&&) = default;
  offset_t& operator=(const offset_t&) = default;
  offset_t& operator=(offset_t&&) = default;
  ~offset_t() = default;

 private:
  friend reader_t;
  Expression _expression;
};

template <typename Context, typename Expression>
auto to_sql_string(Context& context, const offset_t<Expression>& t)
    -> std::string {
  return dynamic_clause_to_sql_string(context, "OFFSET", read.expression(t));
}

template <typename Expression>
struct is_clause<offset_t<Expression>> : public std::true_type {};

template <typename Expression>
struct contains_offset<offset_t<Expression>> : public std::true_type {};

template <typename Statement, typename Expression>
struct consistency_check<Statement, offset_t<Expression>> {
  using type = consistent_t;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename Statement, typename Expression>
struct prepare_check<Statement, offset_t<Expression>> {
  using type = consistent_t;
  constexpr auto operator()() {
    return type{};
  }
};

struct no_offset_t {
  template <typename Statement, typename Arg>
    requires((is_integral<remove_dynamic_t<Arg>>::value or
              is_unsigned_integral<remove_dynamic_t<Arg>>::value) and
             required_tables_of_t<Arg>{}.empty())
  auto offset(this Statement&& self, Arg arg) {
    return new_statement<no_offset_t>(std::forward<Statement>(self),
                                      offset_t<Arg>{std::move(arg)});
  }
};

template <typename Context>
auto to_sql_string(Context&, const no_offset_t&) -> std::string {
  return "";
}

template <typename Statement>
struct consistency_check<Statement, no_offset_t> {
  using type = consistent_t;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename Arg>
    requires((is_integral<remove_dynamic_t<Arg>>::value or
              is_unsigned_integral<remove_dynamic_t<Arg>>::value) and
             required_tables_of_t<Arg>{}.empty())
auto offset(Arg arg) {
  return statement_t<no_offset_t>().offset(std::move(arg));
}

}  // namespace sqlpp
