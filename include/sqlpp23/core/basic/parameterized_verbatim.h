#pragma once

/*
 * Copyright (c) 2013, Roland Bock
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

#include <utility>

#include <sqlpp23/core/operator/enable_as.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
template <typename DataType, typename Expr>
struct parameterized_verbatim_t : public enable_as {
  parameterized_verbatim_t(const Expr expr,
                           std::string verbatim_lhs,
                           std::string verbatim_rhs)
      : _expr(expr),
        _verbatim_lhs(std::move(verbatim_lhs)),
        _verbatim_rhs(std::move(verbatim_rhs)) {}

  parameterized_verbatim_t(const parameterized_verbatim_t&) = default;
  parameterized_verbatim_t(parameterized_verbatim_t&&) = default;
  parameterized_verbatim_t& operator=(const parameterized_verbatim_t&) =
      default;
  parameterized_verbatim_t& operator=(parameterized_verbatim_t&&) = default;
  ~parameterized_verbatim_t() = default;

  Expr _expr;
  std::string _verbatim_lhs, _verbatim_rhs;
};

template <typename DataType, typename Expr>
struct is_clause<parameterized_verbatim_t<DataType, Expr>>
    : public std::true_type {};

template <typename Statement, typename DataType, typename Expr>
struct consistency_check<Statement, parameterized_verbatim_t<DataType, Expr>> {
  using type = consistent_t;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename DataType, typename Expr>
struct data_type_of<parameterized_verbatim_t<DataType, Expr>> {
  // Since we do not know what's going on inside the verbatim, we assume it can
  // be null.
  using type = sqlpp::force_optional_t<DataType>;
};

template <typename DataType, typename Expr>
struct nodes_of<parameterized_verbatim_t<DataType, Expr>> {
  using type = detail::type_vector<Expr>;
};

template <typename Context, typename DataType, typename Expr>
auto to_sql_string(Context& context,
                   const parameterized_verbatim_t<DataType, Expr>& t)
    -> std::string {
  return t._verbatim_lhs + to_sql_string(context, t._expr) + t._verbatim_rhs;
}

template <typename DataType, typename Expr>
auto parameterized_verbatim(std::string lhs, Expr expr, std::string rhs)
    -> parameterized_verbatim_t<DataType, Expr> {
  static_assert(has_data_type<Expr>::value,
                "parameterized_verbatim() requires an expression as argument");
  return {expr, lhs, rhs};
}

}  // namespace sqlpp
