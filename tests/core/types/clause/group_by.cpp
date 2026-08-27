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

void test_group_by() {
  auto v = sqlpp::value(17);
  auto id = test::tab_foo{}.id;
  auto text_nn_d = test::tab_foo{}.text_nn_d;

  using V = decltype(v);
  using Id = decltype(id);
  using TextNnD = decltype(text_nn_d);

  // Static columns are listed as such in known_aggregate_columns_of_t.
  {
    using G = extract_clause_t<decltype(group_by(id))>;
    static_assert(sqlpp::known_aggregate_columns_of<G>::func().size() == 1);
    static_assert(sqlpp::known_aggregate_columns_of<G>::func() ==
                  sqlpp::detail::make_type_info_set<Id>());
    static_assert(sqlpp::known_static_aggregate_columns_of<G>::func() ==
                  sqlpp::detail::make_type_info_set<Id>());
  }
  {
    using G = extract_clause_t<decltype(group_by(id, text_nn_d))>;
    static_assert(sqlpp::known_aggregate_columns_of<G>::func() ==
                  sqlpp::detail::make_type_info_set<Id, TextNnD>());
    static_assert(sqlpp::known_static_aggregate_columns_of<G>::func() ==
                  sqlpp::detail::make_type_info_set<Id, TextNnD>());
  }

  // Dynamic columns are are not listed in known_static_aggregate_columns_of_t.
  {
    using G = extract_clause_t<decltype(group_by(dynamic(true, id)))>;
    static_assert(sqlpp::known_aggregate_columns_of<G>::func() ==
                  sqlpp::detail::make_type_info_set<Id>());
    static_assert(sqlpp::known_static_aggregate_columns_of<G>::func() ==
                  sqlpp::detail::make_type_info_set<>());
  }
  {
    using G = extract_clause_t<decltype(group_by(id, dynamic(true, text_nn_d)))>;
    static_assert(sqlpp::known_aggregate_columns_of<G>::func() ==
                  sqlpp::detail::make_type_info_set<Id, TextNnD>());
    static_assert(sqlpp::known_static_aggregate_columns_of<G>::func() ==
                  sqlpp::detail::make_type_info_set<Id>());
  }

  // Declared columns are listed similar to regular columns.
  {
    using G = extract_clause_t<decltype(group_by(v))>;
    static_assert(sqlpp::known_aggregate_columns_of<G>::func() ==
                  sqlpp::detail::make_type_info_set<V>());
    static_assert(sqlpp::known_static_aggregate_columns_of<G>::func() ==
                  sqlpp::detail::make_type_info_set<V>());
  }
  {
    using G = extract_clause_t<decltype(group_by(dynamic(true, v)))>;
    static_assert(sqlpp::known_aggregate_columns_of<G>::func() ==
                  sqlpp::detail::make_type_info_set<V>());
    static_assert(sqlpp::known_static_aggregate_columns_of<G>::func() ==
                  sqlpp::detail::make_type_info_set<>());
  }
}

int main() {
  void test_group_by();
}
