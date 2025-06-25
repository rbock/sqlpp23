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

#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/tests/core/serialize_helpers.h>
#include <sqlpp23/tests/core/tables.h>

int main(int, char*[]) {
  const auto foo = test::TabFoo{};
  const auto val = foo.id;
  const auto expr = foo.id + 4;

  // Operands are enclosed in parentheses where required.
  SQLPP_COMPARE(val.asc(), "tab_foo.id ASC");
  SQLPP_COMPARE(val.desc(), "tab_foo.id DESC");
  SQLPP_COMPARE(val.order(sqlpp::sort_type::asc), "tab_foo.id ASC");
  SQLPP_COMPARE(val.order(sqlpp::sort_type::desc), "tab_foo.id DESC");

  SQLPP_COMPARE(expr.asc(), "(tab_foo.id + 4) ASC");
  SQLPP_COMPARE(expr.desc(), "(tab_foo.id + 4) DESC");
  SQLPP_COMPARE(expr.order(sqlpp::sort_type::asc), "(tab_foo.id + 4) ASC");
  SQLPP_COMPARE(expr.order(sqlpp::sort_type::desc), "(tab_foo.id + 4) DESC");

  return 0;
}
