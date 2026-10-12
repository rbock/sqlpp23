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

#include <string>
#include <string_view>

#include <sql.h>
#include <sqlext.h>

#include <sqlpp23/odbc/database/exception.h>

namespace sqlpp::odbc::detail {
// Builds an exception from all diagnostic records of the given handle.
// SQLSTATE and native error code are taken from the first record.
inline exception make_exception(std::string_view context,
                                SQLSMALLINT handle_type,
                                SQLHANDLE handle) {
  std::string message{context};
  std::string sql_state;
  SQLINTEGER native_error_code{0};

  for (SQLSMALLINT record_number = 1;; ++record_number) {
    SQLCHAR state[SQL_SQLSTATE_SIZE + 1] = {};
    SQLCHAR text[SQL_MAX_MESSAGE_LENGTH] = {};
    SQLINTEGER native_error{0};
    SQLSMALLINT text_length{0};
    const auto rc =
        SQLGetDiagRec(handle_type, handle, record_number, state, &native_error,
                      text, sizeof(text), &text_length);
    if (rc != SQL_SUCCESS and rc != SQL_SUCCESS_WITH_INFO) {
      break;
    }
    if (record_number == 1) {
      sql_state = reinterpret_cast<const char*>(state);
      native_error_code = native_error;
    }
    message += ": [";
    message += reinterpret_cast<const char*>(state);
    message += "] ";
    message += reinterpret_cast<const char*>(text);
  }

  if (sql_state.empty()) {
    message += ": no diagnostic information available";
  }

  return exception{message, std::move(sql_state), native_error_code};
}

// Throws if the return code indicates failure. SQL_SUCCESS_WITH_INFO is
// treated as success. Returns the return code so that callers can
// distinguish e.g. SQL_NO_DATA where it is a valid outcome.
inline SQLRETURN throw_on_error(SQLRETURN rc,
                                std::string_view context,
                                SQLSMALLINT handle_type,
                                SQLHANDLE handle) {
  if (rc == SQL_SUCCESS or rc == SQL_SUCCESS_WITH_INFO or rc == SQL_NO_DATA) {
    return rc;
  }
  throw make_exception(context, handle_type, handle);
}

// ODBC functions take non-const SQLCHAR* for input strings.
inline SQLCHAR* to_sqlchar_pointer(const std::string& s) {
  return reinterpret_cast<SQLCHAR*>(const_cast<char*>(s.data()));
}
}  // namespace sqlpp::odbc::detail
