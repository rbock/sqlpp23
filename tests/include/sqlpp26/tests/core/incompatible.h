#pragma once

/*
 * Copyright (c) 2025, Roland Bock
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

#ifdef BUILD_WITH_MODULES
import sqlpp23.core;
#else
#include <sqlpp23/core/operator/enable_as.h>
#include <sqlpp23/core/operator/enable_comparison.h>
#include <sqlpp23/core/type_traits.h>
#endif

// Connections need to fail preparing / executing statements that don't pass the
// compatibility check. `sqlpp::test::incompatible(<some value>)` provides a
// value fails the check.

namespace sqlpp::test {
class assert_no_incompatible_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>, "No support for using incompatible expression");
  }
};

template <typename T>
struct incompatible_t : public enable_as, public enable_comparison {
  incompatible_t(T t) : _incompatible(std::move(t)) {}
  incompatible_t(const incompatible_t&) = default;
  incompatible_t(incompatible_t&&) = default;
  incompatible_t& operator=(const incompatible_t&) = default;
  incompatible_t& operator=(incompatible_t&&) = default;
  ~incompatible_t() = default;

  T _incompatible;
};

}  // namespace sqlpp::test

namespace sqlpp {
template <typename Context, typename T>
struct compatibility_check<Context, test::incompatible_t<T>> {
  using type = test::assert_no_incompatible_t;
};

template <typename T>
struct data_type_of<test::incompatible_t<T>> {
  using type = data_type_of_t<T>;
};

template <typename T>
struct nodes_of<test::incompatible_t<T>> {
  using type = detail::type_vector<T>;
};

template <typename T>
struct requires_parentheses<test::incompatible_t<T>> : public requires_parentheses<T> {};

template <typename Context, typename T>
auto to_sql_string(Context& context, const test::incompatible_t<T>& t) -> std::string {
  return to_sql_string(context, t._incompatible);
}

}  // namespace sqlpp

namespace sqlpp::test {
template <typename T>
  requires(values_are_comparable<T, T>::value)
auto incompatible(T t) -> incompatible_t<T> {
  return {std::move(t)};
}

}  // namespace sqlpp::test
