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
#include <sqlpp23/core/basic/star.h>
#include <sqlpp23/core/clause/select_flags.h>
#include <sqlpp23/core/name/create_name_tag.h>
#include <sqlpp23/core/operator/enable_as.h>
#include <sqlpp23/core/operator/enable_comparison.h>
#include <sqlpp23/core/reader.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp::alias {
SQLPP_CREATE_NAME_TAG(count_);
SQLPP_CREATE_NAME_TAG(distinct_count_);
}

namespace sqlpp {
template <typename Flag, typename Expr>
struct count_t : public enable_as,
                 public enable_comparison,
                 public enable_over {
  constexpr count_t(Expr expr) : _expression(std::move(expr)) {}
  constexpr count_t(const count_t&) = default;
  constexpr count_t(count_t&&) = default;
  constexpr count_t& operator=(const count_t&) = default;
  constexpr count_t& operator=(count_t&&) = default;
  ~count_t() = default;

 private:
  friend reader_t;
  Expr _expression;
};

template <typename Flag, typename Expr>
struct is_aggregate_function<count_t<Flag, Expr>> : public std::true_type {};

template <typename Flag, typename Expr>
struct nodes_of<count_t<Flag, Expr>> {
  using type = sqlpp::detail::type_vector<Expr>;
};

template <typename Flag, typename Expr>
struct data_type_of<count_t<Flag, Expr>> {
  using type = integral;
};

template <typename Context, typename Flag, typename Expr>
auto to_sql_string(Context& context, const count_t<Flag, Expr>& t)
    -> std::string {
  return "COUNT(" + to_sql_string(context, Flag()) +
         to_sql_string(context, read.expression(t)) + ")";
}

template <typename T>
  requires(has_data_type_v<T> and not contains_aggregate_function<T>::value)
auto count(T t) -> count_t<no_flag_t, T> {
  return {std::move(t)};
}

inline auto count(star_t s) -> count_t<no_flag_t, star_t> {
  return {std::move(s)};
}

template <typename T>
  requires(has_data_type_v<T> and not contains_aggregate_function<T>::value)
auto count(const distinct_t& /*unused*/, T t) -> count_t<distinct_t, T> {
  return {std::move(t)};
}

}  // namespace sqlpp
