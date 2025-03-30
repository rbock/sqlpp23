/*
 * Copyright (c) 2024, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
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

#include <sqlpp23/tests/core/constraints_helpers.h>

#include <sqlpp23/tests/core/tables.h>

namespace {
SQLPP_CREATE_NAME_TAG(something);

template <typename... Expressions>
concept can_call_avg_with =
    requires(Expressions... expressions) { sqlpp::avg(expressions...); };

template <typename... Expressions>
concept cannot_call_avg_with =
    not(can_call_avg_with<Expressions...>);
}

int main() {
  auto foo = test::TabFoo{};

  static_assert(can_call_avg_with<decltype(foo.id)>);

  static_assert(cannot_call_avg_with<sqlpp::star_t>);
  static_assert(cannot_call_avg_with<decltype(foo.textNnD)>);
  static_assert(cannot_call_avg_with<decltype(foo.id.as(something))>);
  static_assert(cannot_call_avg_with<decltype(foo)>);

  SQLPP_CHECK_STATIC_ASSERT(avg(count(foo.id)),
                            "avg() must not be used on an aggregate function");
  SQLPP_CHECK_STATIC_ASSERT(avg(min(foo.id)),
                            "avg() must not be used on an aggregate function");
  SQLPP_CHECK_STATIC_ASSERT(avg(max(foo.id)),
                            "avg() must not be used on an aggregate function");

  SQLPP_CHECK_STATIC_ASSERT(avg(sqlpp::distinct, count(foo.id)),
                            "avg() must not be used on an aggregate function");
  SQLPP_CHECK_STATIC_ASSERT(avg(sqlpp::distinct, min(foo.id)),
                            "avg() must not be used on an aggregate function");
  SQLPP_CHECK_STATIC_ASSERT(avg(sqlpp::distinct, max(foo.id)),
                            "avg() must not be used on an aggregate function");
}
