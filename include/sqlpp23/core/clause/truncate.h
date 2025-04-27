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

#include <sqlpp23/core/clause/single_table.h>
#include <sqlpp23/core/clause/where.h>
#include <sqlpp23/core/concepts.h>
#include <sqlpp23/core/database/connection.h>
#include <sqlpp23/core/query/statement.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
struct truncate_t {};

template <typename Context>
auto to_sql_string(Context&, const truncate_t&) -> std::string {
  return "TRUNCATE ";
}

template <>
struct is_clause<truncate_t> : public std::true_type {};

template <typename Statement>
struct consistency_check<Statement, truncate_t> {
  using type = consistent_t;
  constexpr auto operator()() {
    return type{};
  }
};

using blank_truncate_t = statement_t<truncate_t, no_single_table_t>;

template <StaticRawTable _Table>
auto truncate(_Table table) {
  return blank_truncate_t{}.single_table(table);
}

}  // namespace sqlpp
