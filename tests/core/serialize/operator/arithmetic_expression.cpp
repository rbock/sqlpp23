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
  const auto expr = foo.id + 4;

  // Operands are enclosed in parenheses where required.
  SQLPP_COMPARE(foo.id + 1, "tab_foo.id + 1");
  SQLPP_COMPARE(foo.id - 1, "tab_foo.id - 1");
  SQLPP_COMPARE(foo.id * 1, "tab_foo.id * 1");
  SQLPP_COMPARE(foo.id / 1, "tab_foo.id / 1");
  SQLPP_COMPARE(foo.id % 1, "tab_foo.id % 1");

  SQLPP_COMPARE(1 + expr, "1 + (tab_foo.id + 4)");
  SQLPP_COMPARE(1 - expr, "1 - (tab_foo.id + 4)");
  SQLPP_COMPARE(1 * expr, "1 * (tab_foo.id + 4)");
  SQLPP_COMPARE(1 / expr, "1 / (tab_foo.id + 4)");
  SQLPP_COMPARE(1 % expr, "1 % (tab_foo.id + 4)");

  SQLPP_COMPARE(expr + 1, "(tab_foo.id + 4) + 1");
  SQLPP_COMPARE(expr - 1, "(tab_foo.id + 4) - 1");
  SQLPP_COMPARE(expr * 1, "(tab_foo.id + 4) * 1");
  SQLPP_COMPARE(expr / 1, "(tab_foo.id + 4) / 1");
  SQLPP_COMPARE(expr % 1, "(tab_foo.id + 4) % 1");

  SQLPP_COMPARE(expr + expr, "(tab_foo.id + 4) + (tab_foo.id + 4)");
  SQLPP_COMPARE(expr - expr, "(tab_foo.id + 4) - (tab_foo.id + 4)");
  SQLPP_COMPARE(expr * expr, "(tab_foo.id + 4) * (tab_foo.id + 4)");
  SQLPP_COMPARE(expr / expr, "(tab_foo.id + 4) / (tab_foo.id + 4)");
  SQLPP_COMPARE(expr % expr, "(tab_foo.id + 4) % (tab_foo.id + 4)");

  // Same for unary expressions.
  SQLPP_COMPARE(-1, "-1");
  SQLPP_COMPARE(-foo.id + 1, "(-foo.id) + 1");
  SQLPP_COMPARE(-expr, "-(tab_foo.id + 4)");

  const auto text = "a";
  const auto text_expr = foo.textNnD + "c";

  // Same for concatenation.
  SQLPP_COMPARE(foo.textNnD + text, "CONCAT(tab_foo.text_nn_d, 'a')");
  SQLPP_COMPARE(text + text_expr, "CONCAT('a', CONCAT(tab_foo.text_nn_d, 'c'))");
  SQLPP_COMPARE(text_expr + text, "CONCAT(CONCAT(tab_foo.text_nn_d, 'c'), 'a')");
  SQLPP_COMPARE(text_expr + text_expr,
                "CONCAT(CONCAT(tab_foo.text_nn_d, 'c'), CONCAT(tab_foo.text_nn_d, 'c'))");

  // Arithmetic expressions can be named with AS
  SQLPP_COMPARE((foo.id + 1).as(sqlpp::alias::a), "(tab_foo.id + 1) AS a");
  SQLPP_COMPARE((foo.id - 1).as(sqlpp::alias::a), "(tab_foo.id - 1) AS a");
  SQLPP_COMPARE((foo.id * 1).as(sqlpp::alias::a), "(tab_foo.id * 1) AS a");
  SQLPP_COMPARE((foo.id / 1).as(sqlpp::alias::a), "(tab_foo.id / 1) AS a");
  SQLPP_COMPARE((foo.id % 1).as(sqlpp::alias::a), "(tab_foo.id % 1) AS a");

  // Arithmetic expressions can be compared
  SQLPP_COMPARE((foo.id + 1) < 17, "(tab_foo.id + 1) < 17");
  SQLPP_COMPARE((foo.id - 1) < 17, "(tab_foo.id - 1) < 17");
  SQLPP_COMPARE((foo.id * 1) < 17, "(tab_foo.id * 1) < 17");
  SQLPP_COMPARE((foo.id / 1) < 17, "(tab_foo.id / 1) < 17");
  SQLPP_COMPARE((foo.id % 1) < 17, "(tab_foo.id % 1) < 17");
  SQLPP_COMPARE(-foo.id < 17, "(-tab_foo.id) < 17");
  SQLPP_COMPARE((foo.textNnD + text) < "z", "CONCAT(tab_foo.text_nn_d, 'a') < 'z'");

  return 0;
}
