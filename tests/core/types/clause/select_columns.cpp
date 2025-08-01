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
#include <sqlpp23/tests/core/tables.h>

namespace {
SQLPP_CREATE_NAME_TAG(cheese);
SQLPP_CREATE_NAME_TAG(cake);

template <typename T>
struct clause_of;

template <typename T>
struct clause_of<sqlpp::statement_t<T>> {
  using type = T;
};
template <typename T>
using clause_of_t = typename clause_of<T>::type;
}  // namespace

void test_select_columns() {
  const auto maybe = true;

  auto v = sqlpp::value("text");
  auto col_int = test::TabFoo{}.id;
  auto col_txt = test::TabFoo{}.textNnD;
  auto col_bool = test::TabFoo{}.boolN;
  auto flag = sqlpp::all;

  // Single column.
  {
    using T = clause_of_t<decltype(select_columns(col_int))>;
    static_assert(not sqlpp::has_name_tag<T>::value, "");
    static_assert(
        std::is_same<sqlpp::data_type_of_t<T>, sqlpp::integral>::value, "");
    static_assert(sqlpp::is_result_clause<T>::value, "");
  }

  // Single column with flag.
  {
    using T = clause_of_t<decltype(select_columns(flag, col_int))>;
    static_assert(not sqlpp::has_name_tag<T>::value, "");
    static_assert(
        std::is_same<sqlpp::data_type_of_t<T>, sqlpp::integral>::value, "");
    static_assert(sqlpp::is_result_clause<T>::value, "");
  }

  // Single dynamic column.
  {
    auto t = select_columns(dynamic(maybe, col_int));
    using T = clause_of_t<decltype(t)>;
    static_assert(not sqlpp::has_name_tag<T>::value, "");
    static_assert(std::is_same<sqlpp::data_type_of_t<T>,
                               std::optional<sqlpp::integral>>::value,
                  "");
    static_assert(sqlpp::is_result_clause<T>::value, "");
  }

  // Single dynamic column with dynamic flag.
  {
    auto t = select_columns(dynamic(maybe, flag), dynamic(maybe, col_int));
    using T = clause_of_t<decltype(t)>;
    static_assert(not sqlpp::has_name_tag<T>::value, "");
    static_assert(std::is_same<sqlpp::data_type_of_t<T>,
                               std::optional<sqlpp::integral>>::value,
                  "");
    static_assert(sqlpp::is_result_clause<T>::value, "");
  }

  // Single aggregate function.
  {
    auto t = select_columns(avg(col_int).as(cheese));
    using T = clause_of_t<decltype(t)>;
    static_assert(not sqlpp::has_name_tag<T>::value, "");
    static_assert(std::is_same<sqlpp::data_type_of_t<T>,
                               std::optional<sqlpp::floating_point>>::value,
                  "");
    static_assert(sqlpp::is_result_clause<T>::value, "");
  }

  // Single dynamic aggregate function.
  {
    auto t = select_columns(dynamic(maybe, avg(col_int).as(cheese)));
    using T = clause_of_t<decltype(t)>;
    static_assert(not sqlpp::has_name_tag<T>::value, "");
    static_assert(std::is_same<sqlpp::data_type_of_t<T>,
                               std::optional<sqlpp::floating_point>>::value,
                  "");
    static_assert(sqlpp::is_result_clause<T>::value, "");
  }

  // Single value.
  {
    using T = clause_of_t<decltype(select_columns(v.as(cheese)))>;
    static_assert(not sqlpp::has_name_tag<T>::value, "");
    static_assert(std::is_same<sqlpp::data_type_of_t<T>, sqlpp::text>::value,
                  "");
    static_assert(sqlpp::is_result_clause<T>::value, "");
  }

  // Single dynamic value.
  {
    using T = clause_of_t<decltype(select_columns(
        dynamic(maybe, v.as(cheese))))>;
    static_assert(not sqlpp::has_name_tag<T>::value, "");
    static_assert(std::is_same<sqlpp::data_type_of_t<T>,
                               std::optional<sqlpp::text>>::value,
                  "");
    static_assert(sqlpp::is_result_clause<T>::value, "");
  }

  // Multiple columns.
  {
    using T = clause_of_t<decltype(select_columns(col_int, col_txt, col_bool))>;
    static_assert(not sqlpp::has_name_tag<T>::value, "");
    static_assert(not sqlpp::has_data_type<T>::value, "");
    static_assert(sqlpp::is_result_clause<T>::value, "");
  }

  // Multiple columns with flag.
  {
    using T = clause_of_t<decltype(select_columns(flag, col_int, col_txt, col_bool))>;
    static_assert(not sqlpp::has_name_tag<T>::value, "");
    static_assert(not sqlpp::has_data_type<T>::value, "");
    static_assert(sqlpp::is_result_clause<T>::value, "");
  }

  // Mixed columns.
  {
    using T = clause_of_t<decltype(select_columns(
        col_int, max(col_txt).as(cake),
        v.as(cheese)))>;
    static_assert(not sqlpp::has_name_tag<T>::value, "");
    static_assert(not sqlpp::has_data_type<T>::value, "");
    static_assert(sqlpp::is_result_clause<T>::value, "");
  }
}

int main() {
  void test_select_columns();
}
