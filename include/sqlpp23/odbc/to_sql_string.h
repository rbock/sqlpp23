#pragma once

/*
 * Copyright (c) 2026, Leander Schulten
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

#include <chrono>
#include <format>
#include <string>

#include <sqlpp23/core/basic/parameter.h>
#include <sqlpp23/core/chrono.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/odbc/database/serializer_context.h>

namespace sqlpp::odbc {
// ODBC uses unnumbered '?' parameter markers.
template <typename DataType, typename NameType>
auto to_sql_string(context_t& context, const parameter_t<DataType, NameType>&)
    -> std::string {
  ++context._count;
  return "?";
}

// Date and time literals use the ODBC escape sequences which every driver
// translates into the syntax of its database.
inline auto to_sql_string(context_t&, const std::chrono::sys_days& t)
    -> std::string {
  return std::format("{{d '{0:%Y-%m-%d}'}}", t);
}

// Note: The ODBC time escape sequence has no sub-second precision,
// fractional seconds are dropped.
inline auto to_sql_string(context_t&, const std::chrono::microseconds& t)
    -> std::string {
  return std::format("{{t '{0:%H:%M:%S}'}}",
                     std::chrono::floor<std::chrono::seconds>(t));
}

template <typename Period>
auto to_sql_string(
    context_t&,
    const std::chrono::time_point<std::chrono::system_clock, Period>& t)
    -> std::string {
  return std::format("{{ts '{0:%Y-%m-%d %H:%M:%S}'}}", t);
}

}  // namespace sqlpp::odbc
