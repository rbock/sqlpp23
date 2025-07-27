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

#include <sqlpp23/tests/core/all.h>

namespace {
SQLPP_CREATE_NAME_TAG(something);
SQLPP_CREATE_NAME_TAG(tab);
}  // namespace

int main() {
  const auto foo = test::TabFoo{};

  // Confirming the required columns of TabBar.
  static_assert(std::is_same<sqlpp::required_insert_columns_of_t<test::TabBar>,
                             sqlpp::detail::type_set<sqlpp::column_t<
                                 test::TabBar, test::TabBar_::BoolNn>>>::value,
                "");

  // -------------------------
  // OK: A consistent select can be used as table.
  // -------------------------
  {
    // minimal example
    auto t = sqlpp::select(sqlpp::value(7).as(something));
    using T = decltype(t.as(tab));
    static_assert(sqlpp::is_table<T>::value, "");
  }

  {
    // parameters are OK
    auto t = sqlpp::select(parameter(foo.id).as(something));
    using T = decltype(t.as(tab));
    static_assert(sqlpp::is_table<T>::value, "");
  }

}
