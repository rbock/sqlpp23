#pragma once

/*
 * Copyright (c) 2026, Roland Bock
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

#include <sqlpp23/core/reader.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
struct verbatim_clause_t {
  verbatim_clause_t(std::string expression) : _expression(std::move(expression)) {}
  verbatim_clause_t(const verbatim_clause_t&) = default;
  verbatim_clause_t(verbatim_clause_t&&) = default;
  verbatim_clause_t& operator=(const verbatim_clause_t&) = default;
  verbatim_clause_t& operator=(verbatim_clause_t&&) = default;
  ~verbatim_clause_t() = default;

 private:
  friend reader_t;
  std::string _expression;
};

template <>
struct is_clause<verbatim_clause_t> : public std::true_type {};

template <typename Statement>
struct consistency_check<Statement, verbatim_clause_t> {
  using type = consistent_t;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename Context>
auto to_sql_string(Context&, const verbatim_clause_t& t) -> std::string {
  return read.expression(t);
}

inline auto verbatim_clause(std::string s) -> verbatim_clause_t {
  return {std::move(s)};
}
}  // namespace sqlpp
