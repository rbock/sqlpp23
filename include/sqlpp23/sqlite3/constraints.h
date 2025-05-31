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
#include <sqlpp23/core/basic/join.h>
#include <sqlpp23/core/basic/parameter.h>
#include <sqlpp23/core/chrono.h>
#include <sqlpp23/core/clause/on_conflict.h>
#include <sqlpp23/core/clause/returning.h>
#include <sqlpp23/core/clause/using.h>
#include <sqlpp23/core/clause/with.h>
#include <sqlpp23/core/database/exception.h>
#include <sqlpp23/core/type_traits.h>
#include <sqlpp23/sqlite3/database/serializer_context.h>
#include <sqlpp23/sqlpp23.h>

#include <cmath>

// Disable some stuff that won't work with sqlite3
// See https://www.sqlite.org/changes.html

namespace sqlpp {
namespace sqlite3 {
class assert_no_any_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>, "Sqlite3: No support for any()");
  }
};

class assert_no_using_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>, "Sqlite3: No support for USING");
  }
};
}  // namespace sqlite3

template <typename Select>
struct compatibility_check<sqlite3::context_t, any_t<Select>> {
  using type = sqlite3::assert_no_any_t;
};

template <typename _Table>
struct compatibility_check<sqlite3::context_t, using_t<_Table>> {
  using type = sqlite3::assert_no_using_t;
};
}  // namespace sqlpp

#if SQLITE_VERSION_NUMBER < 3039000
namespace sqlpp {
namespace sqlite3 {
class assert_no_full_outer_join_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(
        wrong<T...>,
        "Sqlite3: No support for full outer join before version 3.39.0");
  }
};

class assert_no_right_outer_join_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(
        wrong<T...>,
        "Sqlite3: No support for right outer join before version 3.39.0");
  }
};
}  // namespace sqlite3

template <typename Lhs, typename Rhs, typename Condition>
struct compatibility_check<sqlite3::context_t,
                           join_t<Lhs, full_outer_join_t, Rhs, Condition>> {
  using type = sqlite3::assert_no_full_outer_join_t;
};

template <typename Lhs, typename Rhs, typename Condition>
struct compatibility_check<sqlite3::context_t,
                           join_t<Lhs, right_outer_join_t, Rhs, Condition>> {
  using type = sqlite3::assert_no_full_outer_join_t;
};
}  // namespace sqlpp
#endif

#if SQLITE_VERSION_NUMBER < 3035000
namespace sqlpp {
namespace sqlite3 {
class assert_no_returning_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>,
                  "Sqlite3: No support for RETURNING before version 3.35.0");
  }
};

class assert_no_on_conflict_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(
        wrong<T...>,
        "Sqlite3: No full support for ON CONFLICT before version 3.35.0");
  }
};
}  // namespace sqlite3

template <typename... Columns>
struct compatibility_check<sqlite3::context_t, returning_t<Columns...>> {
  using type = sqlite3::assert_no_returning_t;
};

template <typename... Columns>
struct compatibility_check<sqlite3::context_t, on_conflict_t<Columns...>> {
  using type = sqlite3::assert_no_on_conflict_t;
};
}  // namespace sqlpp
#endif

#if SQLITE_VERSION_NUMBER < 3008003
namespace sqlpp {
namespace sqlite3 {
class assert_no_with_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>,
                  "Sqlite3: No support for WITH before version 3.8.3");
  }
};
}  // namespace sqlite3

template <typename... Ctes>
struct compatibility_check<sqlite3::context_t, with_t<Ctes...>> {
  using type = sqlite3::assert_no_with_t;
};
}  // namespace sqlpp
#endif

namespace sqlpp {
namespace sqlite3 {
class assert_no_cast_to_date_time : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>, "Sqlite3: No support for casting to date / time types");
  }
};
}  // namespace sqlite3

template <typename Expression, typename Type>
  requires(std::is_same_v<Type, date> or std::is_same_v<Type, timestamp> or
           std::is_same_v<Type, time>)
struct compatibility_check<sqlite3::context_t, cast_t<Expression, Type>> {
  using type = sqlite3::assert_no_cast_to_date_time;
};

}  // namespace sqlpp

