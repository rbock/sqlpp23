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

int main() {
  const auto foo = test::TabFoo{};
  const auto bar = test::TabBar{};
  const auto aFoo = foo.as(sqlpp::alias::a);
  const auto bFoo = foo.as(sqlpp::alias::b);
  const auto cFoo = foo.as(sqlpp::alias::c);

  const auto x = sqlpp::cte(sqlpp::alias::x).as(select(foo.id).from(foo));
  const auto xa = x.as(sqlpp::alias::a);
  const auto xb = x.as(sqlpp::alias::b);
  const auto y = sqlpp::cte(sqlpp::alias::y).as(select(foo.id).from(foo));

  // Single table
  SQLPP_COMPARE(from(foo), " FROM tab_foo");

  // Single dynamic table
  SQLPP_COMPARE(from(dynamic(true, foo)), " FROM tab_foo");
  SQLPP_COMPARE(from(dynamic(false, foo)), "");

  // Tests with joined tables are mostly covered by join tests.

  // Static joins
  SQLPP_COMPARE(from(foo.cross_join(bar)), " FROM tab_foo CROSS JOIN tab_bar");
  SQLPP_COMPARE(from(foo.join(bar).on(foo.id == bar.id)),
                " FROM tab_foo INNER JOIN tab_bar ON tab_foo.id = tab_bar.id");

  // Dynamic joins
  SQLPP_COMPARE(from(foo.cross_join(dynamic(true, bar))),
                " FROM tab_foo CROSS JOIN tab_bar");
  SQLPP_COMPARE(from(foo.cross_join(dynamic(false, bar))), " FROM tab_foo");

  // Multiple tables
  SQLPP_COMPARE(from(aFoo.join(bFoo)
                         .on(aFoo.id == bFoo.id)
                         .join(cFoo)
                         .on(bFoo.id == cFoo.id)),
                " FROM tab_foo AS a INNER JOIN tab_foo AS b ON a.id = b.id "
                "INNER JOIN tab_foo AS c ON b.id = c.id");

  // CTE
  SQLPP_COMPARE(from(x), " FROM x");
  SQLPP_COMPARE(from(foo.join(x).on(x.id == foo.id)),
                " FROM tab_foo INNER JOIN x ON x.id = tab_foo.id");
  SQLPP_COMPARE(from(x.join(foo).on(x.id == foo.id)),
                " FROM x INNER JOIN tab_foo ON x.id = tab_foo.id");
  SQLPP_COMPARE(from(x.join(y).on(x.id == y.id)),
                " FROM x INNER JOIN y ON x.id = y.id");
  SQLPP_COMPARE(from(xa.join(xb).on(xa.id == xb.id)),
                " FROM x AS a INNER JOIN x AS b ON a.id = b.id");

  return 0;
}
