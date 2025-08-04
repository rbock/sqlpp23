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

#include <string>
#include <string_view>

#include <libpq-fe.h>

#include <sqlpp23/core/database/exception.h>

namespace sqlpp::postgresql {
inline constexpr char fallback[] = "no message";
class connection_exception : public sqlpp::exception {
 public:
  connection_exception(const std::string& what_arg)
      : sqlpp::exception{what_arg} {}

  connection_exception(const char* what_arg)
      : sqlpp::exception{what_arg ? what_arg : fallback} {}
};

class result_exception : public sqlpp::exception {
  ExecStatusType _status;
  std::string _sql_state;
 public:
  result_exception(const std::string& what_arg,
                   ExecStatusType status,
                   const std::string sql_state)
      : sqlpp::exception{what_arg},
        _status{status},
        _sql_state{std::move(sql_state)} {}

  result_exception(const char* what_arg,
                   ExecStatusType status,
                   const std::string sql_state)
      : sqlpp::exception{what_arg ? what_arg : fallback},
        _status{status},
        _sql_state{std::move(sql_state)} {}

  // Returns value of PQresultStatus(_pg_result.get())
  ExecStatusType status() const { return _status; }

  // Returns value of PQresultErrorField(_pg_result.get(), PG_DIAG_SQLSTATE)
  std::string_view sql_state() const { return _sql_state; }
};
}  // namespace sqlpp
