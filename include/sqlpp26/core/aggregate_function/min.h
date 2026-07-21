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

#include <sqlpp23/core/aggregate_function/enable_over.h>
#include <sqlpp23/core/clause/select_flags.h>
#include <sqlpp23/core/name/create_name_tag.h>
#include <sqlpp23/core/operator/enable_as.h>
#include <sqlpp23/core/operator/enable_comparison.h>
#include <sqlpp23/core/reader.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp::alias {
SQLPP_CREATE_NAME_TAG(min_);
SQLPP_CREATE_NAME_TAG(distinct_min_);
}

namespace sqlpp {
template <typename Flag, typename Expr>
struct min_t : public enable_as, public enable_comparison, public enable_over {
  constexpr min_t(Expr expr) : _expression(std::move(expr)) {}
  constexpr min_t(const min_t&) = default;
  constexpr min_t(min_t&&) = default;
  constexpr min_t& operator=(const min_t&) = default;
  constexpr min_t& operator=(min_t&&) = default;
  ~min_t() = default;

 private:
  friend reader_t;
  Expr _expression;
};

template <typename Flag, typename Expr>
struct is_aggregate_function<min_t<Flag, Expr>> : public std::true_type {};

template <typename Flag, typename Expr>
struct nodes_of<min_t<Flag, Expr>> {
  using type = sqlpp::detail::type_vector<Expr>;
};

template <typename Flag, typename Expr>
struct data_type_of<min_t<Flag, Expr>> {
  using type = sqlpp::force_optional_t<data_type_of_t<Expr>>;
};

template <typename Context, typename Flag, typename Expr>
auto to_sql_string(Context& context, const min_t<Flag, Expr>& t)
    -> std::string {
  return "MIN(" + to_sql_string(context, Flag()) +
         to_sql_string(context, read.expression(t)) + ")";
}

template <typename T>
  requires(values_are_comparable<T, T>::value and
           not contains_aggregate_function<T>::value)
auto min(T t) -> min_t<no_flag_t, T> {
  return {std::move(t)};
}

template <typename T>
  requires(values_are_comparable<T, T>::value and
           not contains_aggregate_function<T>::value)
auto min(const distinct_t& /*unused*/, T t) -> min_t<distinct_t, T> {
  return {std::move(t)};
}
}  // namespace sqlpp
