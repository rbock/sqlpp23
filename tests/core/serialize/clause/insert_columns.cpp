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

#include <sqlpp26/tests/core/all.h>

int main(int, char*[]) {
  const auto foo = test::tab_foo{};

  // Without values.
  SQLPP_COMPARE(insert_columns(foo.int_n), " (int_n)");
  SQLPP_COMPARE(insert_columns(foo.int_n, foo.text_nn_d), " (int_n, text_nn_d)");

  // Single column.
  {
    auto i = insert_columns(foo.text_nn_d);
    i.add_values(foo.text_nn_d = "cheese");
    SQLPP_COMPARE(i, " (text_nn_d) VALUES ('cheese')");
  }
  {
    auto i = insert_columns(foo.int_n);
    i.add_values(foo.int_n = 17);
    i.add_values(foo.int_n = sqlpp::default_value);
    i.add_values(dynamic(true, foo.int_n = 42));
    i.add_values(dynamic(false, foo.int_n = 42));
    SQLPP_COMPARE(i, " (int_n) VALUES (17), (DEFAULT), (42), (DEFAULT)");
  }

  {
    auto i = insert_columns(foo.bool_n);
    i.add_values(foo.bool_n = true);
    i.add_values(foo.bool_n = sqlpp::default_value);
    i.add_values(foo.bool_n = std::nullopt);
    SQLPP_COMPARE(i, " (bool_n) VALUES (1), (DEFAULT), (NULL)");
  }

  // Multiple columns.
  {
    auto i = insert_columns(foo.int_n, foo.bool_n, foo.text_nn_d);
    i.add_values(foo.int_n = sqlpp::default_value,
                 foo.bool_n = sqlpp::default_value, foo.text_nn_d = "cheese");
    SQLPP_COMPARE(
        i, " (int_n, bool_n, text_nn_d) VALUES (DEFAULT, DEFAULT, 'cheese')");

    i.add_values(foo.int_n = 17, foo.bool_n = std::nullopt, foo.text_nn_d = "cake");
    SQLPP_COMPARE(i,
                  " (int_n, bool_n, text_nn_d) VALUES (DEFAULT, DEFAULT, "
                  "'cheese'), (17, NULL, 'cake')");
  }

  return 0;
}
