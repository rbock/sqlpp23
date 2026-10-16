#pragma once

/*
 * Copyright (c) 2026
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
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

#include <duckdb.h>

#include <sqlpp23/core/basic/join.h>
#include <sqlpp23/core/basic/parameter.h>
#include <sqlpp23/core/chrono.h>
#include <sqlpp23/core/clause/on_conflict.h>
#include <sqlpp23/core/clause/returning.h>
#include <sqlpp23/core/clause/truncate.h>
#include <sqlpp23/core/clause/union_flags.h>
#include <sqlpp23/core/clause/using.h>
#include <sqlpp23/core/clause/with.h>
#include <sqlpp23/core/database/exception.h>
#include <sqlpp23/core/reader.h>
#include <sqlpp23/core/type_traits.h>
#include <sqlpp23/sqlpp23.h>

#include <cmath>

namespace sqlpp::duck_db {

// DuckDB understands `UNION` (distinct) and `UNION ALL`, both of which are
// produced by the core serializer for the surrounding union_t clause. There is
// no keyword to emit for the distinct flag itself.
inline auto to_sql_string(context_t&, const union_distinct_t&) -> std::string {
  return {};
}

// DuckDB has no TRUNCATE, `DELETE FROM` is the portable way of emptying a table.
inline auto to_sql_string(context_t&, const truncate_t&) -> std::string {
  return "DELETE FROM ";
}

// DuckDB implements the SQL standard IS [NOT] DISTINCT FROM, but it also
// understands sqlite3's short forms. We go with SQL as understood by DuckDB.
template <typename L, typename R>
auto to_sql_string(
    context_t& context,
    const comparison_expression<L, sqlpp::op_is_distinct_from, R>& t)
    -> std::string {
  // Note: Temporary required to enforce parameter ordering.
  auto ret_val = operand_to_sql_string(context, read.lhs(t)) + " IS NOT ";
  return ret_val + operand_to_sql_string(context, read.rhs(t));
}

template <typename L, typename R>
auto to_sql_string(
    context_t& context,
    const comparison_expression<L, sqlpp::op_is_not_distinct_from, R>& t)
    -> std::string {
  auto ret_val = operand_to_sql_string(context, read.lhs(t)) + " IS ";
  return ret_val + operand_to_sql_string(context, read.rhs(t));
}

// Serialize parameters. DuckDB prepares plain `?` placeholders and binds them
// by 1-based position, which is exactly the order in which sqlpp23 walks its
// parameter list.
template <typename DataType, typename NameType>
auto to_sql_string(context_t& context, const parameter_t<DataType, NameType>&)
    -> std::string {
  ++context._count;
  return "?";
}

// DuckDB has no `x'...'` hexadecimal blob literal. Its BLOB literals use the
// `'\xAA'` escape syntax, so a BLOB cannot be represented by the core
// serializer's default (which targets sqlite3 / MySQL).
// See https://duckdb.org/docs/stable/sql/data_types/blob.html
inline auto blob_to_sql_string(const std::span<const uint8_t>& t)
    -> std::string {
  constexpr char hexChars[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                 '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
  auto result = std::string{"'"};
  result.reserve(t.size() * 4 + 8);

  for (const auto c : t) {
    result += "\\x";
    result.push_back(hexChars[c >> 4]);
    result.push_back(hexChars[c & 0x0F]);
  }
  result += "'::BLOB";

  return result;
}

inline auto to_sql_string(context_t&, const std::span<const uint8_t>& t)
    -> std::string {
  return blob_to_sql_string(t);
}

inline auto to_sql_string(context_t& context,
                          const std::vector<uint8_t>& t) -> std::string {
  return to_sql_string(context, std::span<const uint8_t>(t.data(), t.size()));
}

template <std::size_t N>
auto to_sql_string(context_t& context, const std::array<uint8_t, N>& t)
    -> std::string {
  return to_sql_string(context, std::span<const uint8_t>(t.data(), t.size()));
}

// Some special treatment of data types. DuckDB has real DATE / TIME / TIMESTAMP
// types with typed literals, so no conversion functions are needed.
template <typename Period>
auto to_sql_string(
    context_t&,
    const std::chrono::time_point<std::chrono::system_clock, Period>& t)
    -> std::string {
  return std::format("TIMESTAMP '{0:%Y-%m-%d %H:%M:%S}'", t);
}

inline auto to_sql_string(context_t&, const std::chrono::microseconds& t)
    -> std::string {
  return std::format("TIME '{0:%H:%M:%S}'", t);
}

inline auto to_sql_string(context_t&, const std::chrono::sys_days& t)
    -> std::string {
  return std::format("DATE '{0:%Y-%m-%d}'", t);
}

// DuckDB accepts NaN and Infinity for floating point values.
inline auto nan_to_sql_string(context_t&) -> std::string { return "'NaN'"; }

inline auto inf_to_sql_string(context_t&) -> std::string { return "'Inf'"; }

inline auto neg_inf_to_sql_string(context_t&) -> std::string { return "'-Inf'"; }

}  // namespace sqlpp::duck_db
