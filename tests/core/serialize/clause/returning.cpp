/*
 * Copyright (c) 2025, Roland Bock
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

#include <sqlpp23/core/clause/returning.h>
#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/tests/core/serialize_helpers.h>
#include <sqlpp23/tests/core/tables.h>

SQLPP_CREATE_NAME_TAG(cheese);
SQLPP_CREATE_NAME_TAG(cake);

int main(int, char*[]) {
  const auto val = sqlpp::value(17);

  const auto foo = test::TabFoo{};
  const auto bar = test::TabBar{};

  // -----------------------------------------
  // --  returning(<columns>)
  // -----------------------------------------
  // Single column
  SQLPP_COMPARE(returning(foo.doubleN), " RETURNING tab_foo.double_n");

  // Two columns
  SQLPP_COMPARE(returning(foo.doubleN, bar.id),
                " RETURNING tab_foo.double_n, tab_bar.id");

  // All columns of a table
  SQLPP_COMPARE(returning(all_of(foo)),
                " RETURNING tab_foo.id, tab_foo.text_nn_d, tab_foo.int_n, "
                "tab_foo.int_c_n, tab_foo.double_n, tab_foo.u_int_n, "
                "tab_foo.blob_n, tab_foo.bool_n");

  // Optional column
  SQLPP_COMPARE(returning(dynamic(true, bar.id)), " RETURNING tab_bar.id");
  SQLPP_COMPARE(returning(dynamic(false, bar.id)), " RETURNING NULL AS id");

  // Multiple plain columns.
  SQLPP_COMPARE(returning(foo.id, foo.textNnD, foo.boolN),
                " RETURNING tab_foo.id, tab_foo.text_nn_d, tab_foo.bool_n");

  // Single expression
  SQLPP_COMPARE(returning((foo.id + 17).as(cake)),
                " RETURNING (tab_foo.id + 17) AS cake");

  // Single dynamic column.
  SQLPP_COMPARE(returning(dynamic(true, foo.id)), " RETURNING tab_foo.id");
  SQLPP_COMPARE(returning(dynamic(false, foo.id)), " RETURNING NULL AS id");
  SQLPP_COMPARE(returning(dynamic(false, foo.id.as(cake))),
                " RETURNING NULL AS cake");

  // Multiple dynamic columns (this is odd if all are dynamic)
  SQLPP_COMPARE(returning(dynamic(true, foo.id), foo.textNnD, foo.boolN),
                " RETURNING tab_foo.id, tab_foo.text_nn_d, tab_foo.bool_n");
  SQLPP_COMPARE(returning(foo.id, dynamic(true, foo.textNnD), foo.boolN),
                " RETURNING tab_foo.id, tab_foo.text_nn_d, tab_foo.bool_n");
  SQLPP_COMPARE(returning(foo.id, foo.textNnD, dynamic(true, foo.boolN)),
                " RETURNING tab_foo.id, tab_foo.text_nn_d, tab_foo.bool_n");

  SQLPP_COMPARE(returning(dynamic(false, foo.id), foo.textNnD, foo.boolN),
                " RETURNING NULL AS id, tab_foo.text_nn_d, tab_foo.bool_n");
  SQLPP_COMPARE(returning(foo.id, dynamic(false, foo.textNnD), foo.boolN),
                " RETURNING tab_foo.id, NULL AS text_nn_d, tab_foo.bool_n");
  SQLPP_COMPARE(returning(foo.id, foo.textNnD, dynamic(false, foo.boolN)),
                " RETURNING tab_foo.id, tab_foo.text_nn_d, NULL AS bool_n");

  SQLPP_COMPARE(returning(foo.id, dynamic(false, foo.textNnD),
                               dynamic(false, foo.boolN)),
                " RETURNING tab_foo.id, NULL AS text_nn_d, NULL AS bool_n");
  SQLPP_COMPARE(returning(dynamic(false, foo.id), foo.textNnD,
                               dynamic(false, foo.boolN)),
                " RETURNING NULL AS id, tab_foo.text_nn_d, NULL AS bool_n");
  SQLPP_COMPARE(returning(dynamic(false, foo.id),
                               dynamic(false, foo.textNnD), foo.boolN),
                " RETURNING NULL AS id, NULL AS text_nn_d, tab_foo.bool_n");

  // Single value
  SQLPP_COMPARE(returning(val.as(cheese)),
                " RETURNING 17 AS cheese");
  SQLPP_COMPARE(returning((foo.id + 17).as(cake)),
                " RETURNING (tab_foo.id + 17) AS cake");

  // Mixed column and value
  SQLPP_COMPARE(returning(foo.id, val.as(cheese)),
                " RETURNING tab_foo.id, 17 AS cheese");
  SQLPP_COMPARE(returning(val.as(cake), foo.id),
                " RETURNING 17 AS cake, tab_foo.id");

  // Mixed column and dynamic value
  SQLPP_COMPARE(
      returning(foo.id,
                     dynamic(true, val.as(cheese))),
      " RETURNING tab_foo.id, 17 AS cheese");
  SQLPP_COMPARE(
      returning(dynamic(true, val.as(cake)),
                     foo.id),
      " RETURNING 17 AS cake, tab_foo.id");

  SQLPP_COMPARE(
      returning(foo.id,
                     dynamic(false, val.as(cheese))),
      " RETURNING tab_foo.id, NULL AS cheese");
  SQLPP_COMPARE(
      returning(dynamic(false, val.as(cake)),
                     foo.id),
      " RETURNING NULL AS cake, tab_foo.id");

  return 0;
}
