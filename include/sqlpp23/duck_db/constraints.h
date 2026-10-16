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
#include <sqlpp23/core/clause/for_update.h>
#include <sqlpp23/core/clause/on_conflict.h>
#include <sqlpp23/core/clause/returning.h>
#include <sqlpp23/core/clause/using.h>
#include <sqlpp23/core/clause/with.h>
#include <sqlpp23/core/database/exception.h>
#include <sqlpp23/core/type_traits.h>
#include <sqlpp23/duck_db/database/serializer_context.h>
#include <sqlpp23/sqlpp23.h>

#include <cmath>

// DuckDB supports substantially more of the SQL standard than sqlite3 does.
// There is deliberately no version gating here: the connector targets DuckDB
// 1.x, and everything listed below has been available for the whole 1.x line.
//
// Supported, and therefore deliberately not restricted:
//   - FULL OUTER JOIN and RIGHT OUTER JOIN
//   - WITH (including WITH RECURSIVE)
//   - RETURNING on INSERT, UPDATE and DELETE
//   - ON CONFLICT ... DO NOTHING / DO UPDATE
//   - IS [NOT] DISTINCT FROM
//   - casting to DATE, TIME and TIMESTAMP
//   - ANY / ALL quantified comparisons
//   - the USING clause of DELETE
//
// Rejected at compile time because DuckDB does not implement the construct:
//   - the SELECT locking clause, i.e. FOR UPDATE / FOR SHARE

namespace sqlpp {
namespace duck_db {
class assert_no_for_update_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>,
                  "DuckDB: No support for the SELECT locking clause "
                  "(FOR UPDATE / FOR SHARE)");
  }
};
}  // namespace duck_db

template <>
struct compatibility_check<duck_db::context_t, for_update_t> {
  using type = duck_db::assert_no_for_update_t;
};
}  // namespace sqlpp
