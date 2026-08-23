/*
 * Copyright (c) 2013-2015, Roland Bock
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

int Prepared(int, char*[]) {
  sqlpp::mock_db::connection db = sqlpp::mock_db::make_test_connection();
  const auto t = test::tab_bar{};

#if 0
TODO: Do we test something like this in the type tests?
  // empty parameter lists
  {
    using P = sqlpp::make_parameter_list_t<decltype(t.id)>;
    static_assert(nonstatic_data_members_of(^^P, ctx).size() == 0, "type requirement");
  }

  // single parameter
  {
    using P = sqlpp::make_parameter_list_t<decltype(parameter(t.id))>;
    static_assert(data_members_of(^^P, ctx).size() == 1, "type requirement");
    [[maybe_unused]] auto p = P{};
    p.id = 7;
  }

  // single parameter
  {
    using P = sqlpp::make_parameter_list_t<decltype(parameter(t.text_n))>;
    static_assert(nonstatic_data_members_of(^^P, ctx).size() == 1, "type requirement");
    [[maybe_unused]] auto p = P{};
    p.text_n = "cheesecake";
  }

  // single parameter in expression
  {
    using P = sqlpp::make_parameter_list_t<decltype(t.id == parameter(t.id))>;
    static_assert(nonstatic_data_members_of(^^P, ctx).size() == 1, "type requirement");
    [[maybe_unused]] auto p = P{};
    p.id = 7;
  }

  // single parameter in larger expression
  {
    using P = sqlpp::make_parameter_list_t<decltype((t.text_n.like("%") and
                                                     t.id == parameter(t.id)) or
                                                    t.bool_nn != false)>;
    static_assert(nonstatic_data_members_of(^^P, ctx).size() == 1, "type requirement");
    [[maybe_unused]] auto p = P{};
    p.id = 7;
  }

  // three parameters in expression
  {
    using P =
        sqlpp::parameters_of_t<decltype((t.text_n.like(parameter(t.text_n)) and
                                         t.id == parameter(t.id)) or
                                        t.bool_nn != parameter(t.bool_nn))>;
    // parameters
    static_assert(
        std::is_same<
            P, sqlpp::detail::type_vector<
                   decltype(parameter(t.text_n)), decltype(parameter(t.id)),
                   decltype(parameter(t.bool_nn))>>::value,
        "type requirement");
  }

  // OK, fine, now create a named parameter list from an expression
  {
    using Exp = decltype((t.text_n.like(parameter(t.text_n)) and
                          t.id == parameter(t.id)) or
                         t.bool_nn != parameter(t.bool_nn));
    using P = sqlpp::make_parameter_list_t<Exp>;
    P npl;
    static_assert(
        std::is_same<
            sqlpp::parameter_value_t<sqlpp::data_type_of_t<decltype(t.id)>>,
            decltype(npl.id)>::value,
        "type requirement");
    static_assert(
        std::is_same<
            sqlpp::parameter_value_t<sqlpp::data_type_of_t<decltype(t.text_n)>>,
            decltype(npl.text_n)>::value,
        "type requirement");
    static_assert(std::is_same<sqlpp::parameter_value_t<
                                   sqlpp::data_type_of_t<decltype(t.bool_nn)>>,
                               decltype(npl.bool_nn)>::value,
                  "type requirement");
  }

  // Wonderful, now take a look at the parameter list of a select
  {
    auto s = select(all_of(t)).from(t).where(
        (t.text_n.like(parameter(t.text_n)) and t.id == parameter(t.id)) or
        t.bool_nn != parameter(t.bool_nn));
    auto p = db.prepare(s);
    p.parameters.id = 7;
    using S = decltype(s);
    using P = sqlpp::make_parameter_list_t<S>;
    P npl;

    static_assert(
        std::is_same<
            sqlpp::parameter_value_t<sqlpp::data_type_of_t<decltype(t.id)>>,
            decltype(npl.id)>::value,
        "type requirement");
    static_assert(
        std::is_same<
            sqlpp::parameter_value_t<sqlpp::data_type_of_t<decltype(t.text_n)>>,
            decltype(npl.text_n)>::value,
        "type requirement");
    static_assert(std::is_same<sqlpp::parameter_value_t<
                                   sqlpp::data_type_of_t<decltype(t.bool_nn)>>,
                               decltype(npl.bool_nn)>::value,
                  "type requirement");
    npl.id = 7;
    auto x = npl;
    x = npl;
    std::cerr << x.id << std::endl;
    x = decltype(npl)();
    std::cerr << x.id << std::endl;
  }
#endif

  // Can we prepare a query without parameters?
  {
    auto ps = db.prepare(select(all_of(t)).from(t).where((t.text_n.like("%"))));
    for (const auto& row : db(ps)) {
      std::cerr << row.id << std::endl;
    }
  }

  return 0;
}
