#pragma once

/*
 * Copyright (c) 2013 - 2016, Roland Bock
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

#ifdef SQLPP_USE_SQLCIPHER
#include <sqlcipher/sqlite3.h>
#else
#include <sqlite3.h>
#endif
#include <sqlpp23/core/basic/join.h>
#include <sqlpp23/core/basic/parameter.h>
#include <sqlpp23/core/chrono.h>
#include <sqlpp23/core/clause/on_conflict.h>
#include <sqlpp23/core/clause/returning.h>
#include <sqlpp23/core/clause/using.h>
#include <sqlpp23/core/clause/with.h>
#include <sqlpp23/core/database/exception.h>
#include <sqlpp23/core/type_traits.h>
#include <sqlpp23/sqlite3/database/connection.h>
#include <sqlpp23/sqlpp23.h>

#include <cmath>

namespace sqlpp::sqlite3 {
inline auto to_sql_string(context_t&, const union_distinct_t&) -> std::string {
  return {};
}

inline auto to_sql_string(context_t&, const truncate_t&) -> std::string {
  return "DELETE FROM ";
}

template <typename L, typename R>
auto to_sql_string(context_t& context,
                   const comparison_expression<L, sqlpp::op_is_distinct_from, R>& t)
    -> std::string {
  // Note: Temporary required to enforce parameter ordering.
  auto ret_val = operand_to_sql_string(context, t._l) + " IS NOT ";
  return ret_val + operand_to_sql_string(context, t._r);
}

template <typename L, typename R>
auto to_sql_string(context_t& context,
                   const comparison_expression<L, sqlpp::op_is_not_distinct_from, R>& t)
    -> std::string {
  // Note: Temporary required to enforce parameter ordering.
  auto ret_val = operand_to_sql_string(context, t._l) + " IS ";
  return ret_val + operand_to_sql_string(context, t._r);
}

// Serialize parameters
template <typename DataType, typename NameType>
auto to_sql_string(context_t& context, const parameter_t<DataType, NameType>&)
    -> std::string {
  return "?" + std::to_string(++context._count);
}

// Some special treatment of data types
template <typename Period>
auto to_sql_string(
    context_t&,
    const std::chrono::time_point<std::chrono::system_clock, Period>& t)
    -> std::string {
  return std::format("DATETIME('{0:%Y-%m-%d %H:%M:%S}', 'subsec')", t);
}

inline auto to_sql_string(context_t&, const std::chrono::microseconds& t)
    -> std::string {
  return std::format("TIME('{0:%H:%M:%S}', 'subsec')", t);
}

inline auto to_sql_string(context_t&, const std::chrono::sys_days& t)
    -> std::string {
  return std::format("DATE('{0:%Y-%m-%d}')", t);
}

inline auto nan_to_sql_string(context_t&) -> std::string {
  return "'NaN'";
}

inline auto inf_to_sql_string(context_t&) -> std::string {
  return "'Inf'";
}

inline auto neg_inf_to_sql_string(context_t&) -> std::string {
  return "'-Inf'";
}

}  // namespace sqlpp::sqlite3
