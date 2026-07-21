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

#include <sqlpp23/core/operator/enable_as.h>
#include <sqlpp23/core/operator/enable_comparison.h>
#include <sqlpp23/core/reader.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
template <typename Expr>
struct over_t : public enable_as, public enable_comparison {
  constexpr over_t(Expr expr) : _expression(expr) {}
  constexpr over_t(const over_t&) = default;
  constexpr over_t(over_t&&) = default;
  constexpr over_t& operator=(const over_t&) = default;
  constexpr over_t& operator=(over_t&&) = default;
  ~over_t() = default;

 private:
  friend reader_t;
  Expr _expression;
};

template <typename Expr>
struct nodes_of<over_t<Expr>> : public nodes_of<Expr> {};

template <typename Expr>
struct data_type_of<over_t<Expr>> : public data_type_of<Expr> {};

template <typename Context, typename Expr>
auto to_sql_string(Context& context, const over_t<Expr>& t) -> std::string {
  return operand_to_sql_string(context, read.expression(t)) + " OVER()";
}

template <typename Expr>
  requires(is_aggregate_function<Expr>::value)
auto over(Expr t) -> over_t<Expr> {
  return {std::move(t)};
}

}  // namespace sqlpp
