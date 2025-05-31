#pragma once

/**
 * Copyright © 2025, Roland Bock
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

#include <cstdlib>
#include <memory>

#include <libpq-fe.h>
#include <pg_config.h>

#include <sqlpp23/postgresql/database/exception.h>

namespace sqlpp::postgresql {

class pg_result_t {
  std::unique_ptr<PGresult, void (*)(PGresult*)> _pg_result{nullptr, PQclear};
  public:
  pg_result_t() = default;

  pg_result_t(PGresult* pg_result)
      : _pg_result{std::move(pg_result), PQclear} {
    switch(PQresultStatus(_pg_result.get())) {
      case PGRES_TUPLES_OK:
      case PGRES_COMMAND_OK:
      case PGRES_SINGLE_TUPLE:
        return;
      default:
        throw result_exception{
            PQresultErrorMessage(_pg_result.get()),
            PQresultStatus(_pg_result.get()),
            PQresultErrorField(_pg_result.get(), PG_DIAG_SQLSTATE)};
    }
  }

  pg_result_t(const pg_result_t&) = delete;
  pg_result_t(pg_result_t&&) = default;
  pg_result_t& operator=(const pg_result_t&) = delete;
  pg_result_t& operator=(pg_result_t&&) = default;
  ~pg_result_t() = default;

  PGresult* get() const { return _pg_result.get(); }

  size_t affected_rows() {
    return std::strtoull(PQcmdTuples(_pg_result.get()), nullptr, 10);
  }

};
}  // namespace sqlpp::postgresql
