#pragma once

/*
 * Copyright (c) 2013 - 2015, Roland Bock
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

#include <sqlpp23/mysql/database/connection.h>
#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/core/operator/comparison_expression.h>
#include <sqlpp23/core/type_traits/data_type.h>

namespace sqlpp::mysql {
template <typename L, typename R>
auto to_sql_string(mysql::context_t& context,
                   const comparison_expression<L, sqlpp::op_is_distinct_from, R>& t)
    -> std::string {
  // Note: Temporary required to enforce parameter ordering.
  auto ret_val = "NOT (" + operand_to_sql_string(context, t._l) + " <=> ";
  return ret_val + operand_to_sql_string(context, t._r) + ")";
}

template <typename L, typename R>
auto to_sql_string(mysql::context_t& context,
                   const comparison_expression<L, sqlpp::op_is_not_distinct_from, R>& t)
    -> std::string {
  // Note: Temporary required to enforce parameter ordering.
  auto ret_val = operand_to_sql_string(context, t._l) + " <=> ";
  return ret_val + operand_to_sql_string(context, t._r);
}

inline auto to_sql_string(mysql::context_t&, const insert_default_values_t&)
    -> std::string {
  return " () VALUES()";
}

inline auto quoted_name_to_sql_string(mysql::context_t&,
                                      const std::string_view& name)
    -> std::string {
  return '`' + std::string(name) + '`';
}

inline auto data_type_to_sql_string(mysql::context_t&,
                          const sqlpp::integral&) -> std::string {
  return "SIGNED INTEGER";
}

inline auto data_type_to_sql_string(mysql::context_t&,
                          const sqlpp::unsigned_integral&) -> std::string {
  return "UNSIGNED INTEGER";
}

inline auto data_type_to_sql_string(mysql::context_t&,
                          const sqlpp::floating_point&) -> std::string {
  return "DOUBLE";
}

inline auto data_type_to_sql_string(mysql::context_t&,
                          const sqlpp::text&) -> std::string {
  return "CHAR";
}

inline auto data_type_to_sql_string(mysql::context_t&,
                          const sqlpp::blob&) -> std::string {
  return "BINARY";
}

inline auto data_type_to_sql_string(mysql::context_t&,
                          const sqlpp::timestamp&) -> std::string {
  return "DATETIME";
}

}  // namespace sqlpp
