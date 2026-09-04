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

void test_no_insert_value_list() {
  const auto foo = test::tab_foo{};
  expect_basic_consistency_fails<
      decltype(insert_into(foo)),
      "insert values required, e.g. set(...) or default_values()">();
}

void test_required_insert_columns_of() {
  const auto foo = test::tab_foo{};
  const auto bar = test::tab_bar{};

  static_assert(sqlpp::required_insert_columns_of<test::tab_foo>::func().empty(), "");
  static_assert(not sqlpp::required_insert_columns_of<test::tab_bar>::func().empty(), "");

  {
    using I = decltype(insert_into(foo));

    static_assert(sqlpp::required_insert_columns_of<I>::func() ==
                  sqlpp::required_insert_columns_of<test::tab_foo>::func());
  }

  {
    using I = decltype(insert_into(bar));

    static_assert(sqlpp::required_insert_columns_of<I>::func() ==
                  sqlpp::required_insert_columns_of<test::tab_bar>::func());
  }
}

void test_insert_set() {
  const auto foo = test::tab_foo{};
  const auto bar = test::tab_bar{};

  // Confirming the required columns of tab_bar.
  static_assert(sqlpp::required_insert_columns_of<test::tab_bar>::func() == 
                             sqlpp::detail::make_type_info_set<decltype(bar.bool_nn)>());

  // Test nodes_of
  {
    using I = extract_clause_t<decltype(insert_set(bar.bool_nn = true))>;
    using A = decltype(bar.bool_nn = true);
    static_assert(std::is_same<sqlpp::nodes_of_t<I>,
                               sqlpp::detail::type_vector<A>>::value,
                  "");
  }

  // insert_into(table).set(<all required tables>) is consistent
  {
    using I = decltype(insert_into(bar).set(bar.bool_nn = true));
    consteval {
      I::check_basic_consistency();
    }
  }

  // insert_into(table).set(<all required tables> plus some more) is consistent
  {
    using I = decltype(insert_into(bar).set(bar.int_n = sqlpp::default_value,
                                            bar.bool_nn = true));
    consteval {
      I::check_basic_consistency();
    }
  }

  // insert_into(tableA).set(assignments for tableB) not consistent
  {
    expect_basic_consistency_fails<
        decltype(insert_into(foo).set(bar.int_n = sqlpp::default_value,
                                      bar.bool_nn = true)),
        "at least one insert assignment requires a table which is otherwise not known in the statement">();
  }

  // insert_into(tableA).set(missing required assignments) not consistent
  {
    expect_basic_consistency_fails<
        decltype(insert_into(bar).set(bar.int_n = sqlpp::default_value)),
        "at least one required column is missing in insert assignments">();
  }
  {
    expect_basic_consistency_fails<
        decltype(insert_into(bar).set(bar.int_n = sqlpp::default_value,
                                      dynamic(true, bar.bool_nn = true))),
        "at least one required column is missing in insert assignments">();
  }
}

void test_insert_columns() {
  const auto foo = test::tab_foo{};
  const auto bar = test::tab_bar{};

  // Test nodes_of
  {
    using I = extract_clause_t<decltype(insert_columns(bar.int_n, bar.bool_nn))>;
    using A = decltype(bar.int_n);
    using B = decltype(bar.bool_nn);
    static_assert(std::is_same<sqlpp::nodes_of_t<I>,
                               sqlpp::detail::type_vector<A, B>>::value);
  }

  // insert_into(tableA).columns(decent set of columns from tableA) is
  // consistent
  {
    using I = decltype(insert_into(bar).columns(bar.int_n, bar.bool_nn));
      I::check_basic_consistency();
  }

  // insert_into(tableA).columns(decent set of columns from tableB) is not
  // consistent
  {
    expect_basic_consistency_fails<
        decltype(insert_into(foo).columns(bar.int_n, bar.bool_nn)),
        "at least one column requires a table which is "
        "otherwise not known in the statement">();
  }

  // insert_into(tableA).columns(missing required static columns) is not
  // consistent
  {
    expect_basic_consistency_fails<
        decltype(insert_into(bar).columns(bar.int_n)),
        "at least one required column is missing in columns()">();
  }
}

int main() {
  void test_no_insert_value_list();
  void test_all_columns_have_default_values();
  void test_insert_set();
  void test_insert_columns();
}
