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

#ifdef SQLPP_USE_SQLCIPHER
#include <sqlcipher/sqlite3.h>
#else
#include <sqlite3.h>
#endif
#include <sqlpp26/core/basic/join.h>
#include <sqlpp26/core/basic/parameter.h>
#include <sqlpp26/core/chrono.h>
#include <sqlpp26/core/clause/on_conflict.h>
#include <sqlpp26/core/clause/returning.h>
#include <sqlpp26/core/clause/using.h>
#include <sqlpp26/core/clause/with.h>
#include <sqlpp26/core/database/exception.h>
#include <sqlpp26/core/type_traits.h>
#include <sqlpp26/sqlite3/database/serializer_context.h>
#include <sqlpp26/sqlpp26.h>

// Disable some stuff that won't work with sqlite3
// See https://www.sqlite.org/changes.html

namespace sqlpp {

template <typename Select>
constexpr void check_compatibility(type_v<sqlite3::context_t>, type_v<any_t<Select>>) {
  throw std::domain_error("Sqlite3: No support for any()");
};

template <typename _Table>
constexpr void check_compatibility(type_v<sqlite3::context_t>, type_v<using_t<_Table>>) {
  throw std::domain_error("Sqlite3: No support for USING");
};

#if SQLITE_VERSION_NUMBER < 3039000
template <typename Lhs, typename Rhs, typename Condition>
constexpr void check_compatibility(type_v<sqlite3::context_t>,
                           type_v<join_t<Lhs, full_outer_join_t, Rhs, Condition>>) {
  throw std::domain_error("Sqlite3: No support for full outer join before version 3.39.0");
};

template <typename Lhs, typename Rhs, typename Condition>
constexpr void check_compatibility(type_v<sqlite3::context_t>,
                           type_v<join_t<Lhs, right_outer_join_t, Rhs, Condition>>) {
  throw std::domain_error("Sqlite3: No support for right outer join before version 3.39.0");
};
#endif

#if SQLITE_VERSION_NUMBER < 3035000
template <typename... Columns>
constexpr void check_compatibility(type_v<sqlite3::context_t>, type_v<returning_t<Columns...>>) {
  throw std::domain_error("Sqlite3: No support for RETURNING before version 3.35.0");
};

template <typename... Columns>
constexpr void check_compatibility(type_v<sqlite3::context_t>, type_v<on_conflict_t<Columns...>>) {
  throw std::domain_error("Sqlite3: No full support for ON CONFLICT before version 3.35.0");
};
#endif

#if SQLITE_VERSION_NUMBER < 3008003
template <typename... Ctes>
constexpr void check_compatibility(type_v<sqlite3::context_t>, type_v<with_t<Ctes...>>) {
  throw std::domain_error("Sqlite3: No support for WITH before version 3.8.3");
};
#endif

template <typename Expression, typename Type>
  requires(sqlpp::is_date_v<Type> or sqlpp::is_timestamp_v<Type> or
           sqlpp::is_time_of_day_v<Type>)
constexpr void check_compatibility(type_v<sqlite3::context_t>,
                                   type_v<cast_t<Expression, Type>>) {
  throw std::domain_error("Sqlite3: No support for casting to date / time types");
}

}  // namespace sqlpp

