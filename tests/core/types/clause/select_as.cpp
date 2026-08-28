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
#include "sqlpp26/core/type_traits/data_type.h"

namespace {
template <typename T, typename V>
using is_same_type = std::is_same<sqlpp::data_type_of_t<T>, V>;

template <typename T, typename V>
using is_select_column_same_type =
    std::is_same<sqlpp::select_column_data_type_of_t<T>, V>;
}  // namespace

void test_select_as() {
  auto v_not_null = sqlpp::value(7).as<"always">();
  auto v_maybe_null = sqlpp::value(std::optional{7}).as<"sometimes">();

  using DataType = int;
  using OptDataType = std::optional<int>;

  using ResultDataType = int64_t;
  using OptResultDataType = std::optional<int64_t>;

  // SINGLE VALUE, NOT NULL
  {
    auto s = select(v_not_null);
    auto vs = value(s);
    // A select of a single value can be used as a value.
    static_assert(is_same_type<decltype(s), sqlpp::no_value_t>());
    static_assert(is_same_type<decltype(vs), DataType>());

    // A select of a single value has a value but no name.
    static_assert(not sqlpp::has_name_v<decltype(s)>);
    static_assert(is_same_type<decltype(vs), DataType>());

    // A select of a single value can be named and used as a pseudo table
    static_assert(sqlpp::has_name_v<decltype(s.as<"something">())>);
    static_assert(sqlpp::is_table<decltype(s.as<"something">())>::value);

    // A named select of a single value has no value.
    static_assert(
        not sqlpp::has_data_type<decltype(s.as<"something">())>::value);

    // The column of a single-value pseudo table can be used as named value
    static_assert(
        sqlpp::is_column<decltype(s.as<"something">().always)>::value);
    static_assert(sqlpp::has_name_v<decltype(s.as<"something">().always)>);
    static_assert(
        is_same_type<decltype(s.as<"something">().always), ResultDataType>());

    static_assert(
        sqlpp::has_name_v<decltype(s.as<"something">().always.as<"foo">())>);
    static_assert(sqlpp::select_column_has_name<
                  decltype(s.as<"something">().always.as<"foo">())>::value);
    static_assert(is_select_column_same_type<
                  decltype(s.as<"something">().always.as<"foo">()),
                  ResultDataType>());
  }

  // SINGLE VALUE, MAYBE NULL
  {
    auto s = select(v_maybe_null);
    auto vs = value(s);

    // A select of a single value can be used as a value.
    static_assert(is_same_type<decltype(s), sqlpp::no_value_t>());
    static_assert(is_same_type<decltype(vs), OptDataType>());

    // A select of a single value has a value but no name.
    static_assert(not sqlpp::has_name_v<decltype(s)>);
    static_assert(is_same_type<decltype(vs), OptDataType>());

    // A select of a single value can be named and used as a pseudo table
    static_assert(sqlpp::has_name_v<decltype(s.as<"something">())>);
    static_assert(sqlpp::is_table<decltype(s.as<"something">())>::value);

    // A named select of a single value has no value.
    static_assert(
        not sqlpp::has_data_type<decltype(s.as<"something">())>::value);

    // The column of a single-value pseudo table can be used as named value
    static_assert(
        sqlpp::is_column<decltype(s.as<"something">().sometimes)>::value);
    static_assert(sqlpp::has_name_v<decltype(s.as<"something">().sometimes)>);
    static_assert(is_same_type<decltype(s.as<"something">().sometimes),
                               OptResultDataType>());

    // The column of a single-value pseudo table can be renamed and used as
    // named value
    static_assert(
        sqlpp::has_name_v<decltype(s.as<"something">().sometimes.as<"foo">())>);
    static_assert(sqlpp::select_column_has_name<
                  decltype(s.as<"something">().sometimes.as<"foo">())>::value);
    static_assert(is_select_column_same_type<
                  decltype(s.as<"something">().sometimes.as<"foo">()),
                  OptResultDataType>());
  }

  // SINGLE PARAMETER, NOT NULL
  {
    auto p = sqlpp::parameter<"always", int>();
    auto s = select(p.as<"always">());
    auto vs = value(s);

    using P = decltype(p);
    using S = decltype(s);

    // Parameters are exposed by select_as.
    static_assert(std::is_same<sqlpp::nodes_of_t<decltype(s.as<"something">())>,
                               sqlpp::detail::type_vector<S>>::value);
    static_assert(
        std::is_same<sqlpp::parameters_of_t<decltype(s.as<"something">())>,
                     sqlpp::detail::type_vector<P>>::value);

    // A select of a single value can be used as a value.
    static_assert(is_same_type<decltype(s), sqlpp::no_value_t>());
    static_assert(is_same_type<decltype(vs), ResultDataType>());

    // A select of a single value has a value but no name.
    static_assert(not sqlpp::has_name_v<decltype(s)>);
    static_assert(is_same_type<decltype(vs), ResultDataType>());

    // A select of a single value can be named and used as a pseudo table
    static_assert(sqlpp::has_name_v<decltype(s.as<"something">())>);
    static_assert(sqlpp::is_table<decltype(s.as<"something">())>::value);

    // The column of a single-value pseudo table can be used as named value
    static_assert(
        sqlpp::is_column<decltype(s.as<"something">().always)>::value);
    static_assert(sqlpp::has_name_v<decltype(s.as<"something">().always)>);
    static_assert(
        is_same_type<decltype(s.as<"something">().always), ResultDataType>());

    // The column of a single-value pseudo table can be renamed and used as
    // named value
    static_assert(
        sqlpp::has_name_v<decltype(s.as<"something">().always.as<"foo">())>);
    static_assert(sqlpp::select_column_has_name<
                  decltype(s.as<"something">().always.as<"foo">())>::value);
    static_assert(is_select_column_same_type<
                  decltype(s.as<"something">().always.as<"foo">()),
                  ResultDataType>());
  }

  // MULTIPLE VALUES
  {
    auto s = select(v_not_null, v_maybe_null);

    // A select of multiple values can not be used as a value.
    static_assert(not sqlpp::has_data_type<decltype(s)>::value);

    // A select of multiple values can be named and used as a named value.
    static_assert(sqlpp::has_name_v<decltype(s.as<"something">())>);
    static_assert(
        not sqlpp::has_data_type<decltype(s.as<"something">())>::value);

    // A select of multiple values can be named and used as a pseudo table
    static_assert(sqlpp::is_table<decltype(s.as<"table">())>::value);

    // The column of a multi-value pseudo table can be used as named value
    static_assert(sqlpp::is_column<decltype(s.as<"table">().always)>::value);
    static_assert(sqlpp::is_column<decltype(s.as<"table">().sometimes)>::value);

    static_assert(sqlpp::has_name_v<decltype(s.as<"table">().always)>);
    static_assert(sqlpp::has_name_v<decltype(s.as<"table">().sometimes)>);

    static_assert(
        is_same_type<decltype(s.as<"table">().always), ResultDataType>());
    static_assert(
        is_same_type<decltype(s.as<"table">().sometimes), OptResultDataType>());

    // The column of a multi-value pseudo table can be renamed and used as named
    // value
    static_assert(
        sqlpp::has_name_v<decltype(s.as<"table">().always.as<"foo">())>);
    static_assert(
        sqlpp::has_name_v<decltype(s.as<"table">().sometimes.as<"foo">())>);

    static_assert(sqlpp::select_column_has_name<
                  decltype(s.as<"table">().always.as<"foo">())>::value);
    static_assert(sqlpp::select_column_has_name<
                  decltype(s.as<"table">().sometimes.as<"foo">())>::value);

    static_assert(
        is_select_column_same_type<decltype(s.as<"table">().always.as<"foo">()),
                                   ResultDataType>());
    static_assert(is_select_column_same_type<
                  decltype(s.as<"table">().sometimes.as<"foo">()),
                  OptResultDataType>());
  }
}

int main() {
  test_select_as();
}
