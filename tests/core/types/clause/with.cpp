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

template <typename SelectWith>
struct extract_with;

template <typename... Ctes>
struct extract_with<sqlpp::statement_t<sqlpp::with_t<Ctes...>>> {
  using type = sqlpp::with_t<Ctes...>;
};

template <typename SelectWith>
using extract_with_t = typename extract_with<SelectWith>::type;

void test_with() {
  const auto foo = test::tab_foo{};

  // ctes referencing other CTEs require such ctes. `have_correct_dependencies`
  // checks that.
  {
    auto basic = sqlpp::cte<"basic">().as(select(foo.id).from(foo));
    using Basic = decltype(basic);

    auto referencing =
        sqlpp::cte<"referencing">().as(select(basic.id).from(basic));
    using Referencing = decltype(referencing);

    // Simple good cases.
    static_assert(sqlpp::have_correct_cte_dependencies<Basic>());
    static_assert(
        sqlpp::have_correct_cte_dependencies<sqlpp::dynamic_t<Basic>>());
    static_assert(sqlpp::have_correct_cte_dependencies<Basic, Referencing>());
    static_assert(
        sqlpp::have_correct_cte_dependencies<Basic,
                                             sqlpp::dynamic_t<Referencing>>());

    // The library has no way of knowing if `Basic` and `Referencing` are
    // dynamically added in the correct combinations
    // (`Basic` has to be present if `Referencing` is added). It has to assume
    // that the library user knows what they are doing.
    static_assert(
        sqlpp::have_correct_cte_dependencies<sqlpp::dynamic_t<Basic>,
                                             sqlpp::dynamic_t<Referencing>>());

    // `Referencing` requires the cte it references.
    static_assert(not sqlpp::have_correct_cte_dependencies<Referencing>());
    static_assert(not sqlpp::have_correct_cte_dependencies<
                  sqlpp::dynamic_t<Referencing>>());

    // `Referencing` has to mentioned after the cte it references.
    static_assert(
        not sqlpp::have_correct_cte_dependencies<Referencing, Basic>());
    static_assert(
        not sqlpp::have_correct_cte_dependencies<sqlpp::dynamic_t<Referencing>,
                                                 Basic>());
    static_assert(
        not sqlpp::have_correct_cte_dependencies<sqlpp::dynamic_t<Referencing>,
                                                 sqlpp::dynamic_t<Basic>>());

    // `Referencing` statically requires the cte it references. It is not
    // sufficient to have a dynamic `Basic` cte.
    static_assert(sqlpp::have_correct_cte_dependencies<sqlpp::dynamic_t<Basic>,
                                                       Referencing>());
    static_assert(
        not sqlpp::have_correct_static_cte_dependencies<sqlpp::dynamic_t<Basic>,
                                                        Referencing>());
  }

  // ctes dynamically referencing other CTEs require such ctes dynamically.
  // `have_correct_dependencies` checks that.
  {
    auto basic = sqlpp::cte<"basic">().as(select(foo.id).from(foo));
    using Basic = decltype(basic);

    auto referencing = sqlpp::cte<"referencing">().as(
        select(dynamic(true, basic.id))
            .from(foo.join(dynamic(true, basic)).on(foo.id == basic.id)));
    using Referencing = decltype(referencing);

    // Simple good cases.
    static_assert(sqlpp::have_correct_cte_dependencies<Basic>());
    static_assert(
        sqlpp::have_correct_cte_dependencies<sqlpp::dynamic_t<Basic>>());
    static_assert(sqlpp::have_correct_cte_dependencies<Basic, Referencing>());
    static_assert(
        sqlpp::have_correct_cte_dependencies<Basic,
                                             sqlpp::dynamic_t<Referencing>>());

    // The library has no way of knowing if `Basic` and `Referencing` are
    // dynamically added in the correct combinations
    // (`Basic` has to be present if `Referencing` is added). It has to assume
    // that the library user knows what they are doing.
    static_assert(
        sqlpp::have_correct_cte_dependencies<sqlpp::dynamic_t<Basic>,
                                             sqlpp::dynamic_t<Referencing>>());

    // `Referencing` requires the cte it references.
    static_assert(not sqlpp::have_correct_cte_dependencies<Referencing>());
    static_assert(not sqlpp::have_correct_cte_dependencies<
                  sqlpp::dynamic_t<Referencing>>());

    // `Referencing` has to mentioned after the cte it references.
    static_assert(
        not sqlpp::have_correct_cte_dependencies<Referencing, Basic>());
    static_assert(
        not sqlpp::have_correct_cte_dependencies<sqlpp::dynamic_t<Referencing>,
                                                 Basic>());
    static_assert(
        not sqlpp::have_correct_cte_dependencies<sqlpp::dynamic_t<Referencing>,
                                                 sqlpp::dynamic_t<Basic>>());

    // `Referencing` dynamically requires the cte it references. It is
    // sufficient to have a dynamic `Basic` cte.
    static_assert(sqlpp::have_correct_cte_dependencies<sqlpp::dynamic_t<Basic>,
                                                       Referencing>());
    static_assert(
        sqlpp::have_correct_static_cte_dependencies<sqlpp::dynamic_t<Basic>,
                                                    Referencing>());
  }

  // Self-referencing CTEs do not necessarily require other ctes.
  // `have_correct_dependencies` checks that.
  {
    auto basic = sqlpp::cte<"basic">().as(select(foo.id).from(foo));
    using Basic = decltype(basic);

    auto recursive_base =
        sqlpp::cte<"recursive">().as(select(sqlpp::value(1).as<"a">()));
    auto recursive =
        recursive_base.union_all(select((recursive_base.a + 1).as<"a">())
                                     .from(recursive_base)
                                     .where(recursive_base.a <= 10));
    using Recursive = decltype(recursive);

    // Simple good cases.
    static_assert(sqlpp::have_correct_cte_dependencies<Basic>());
    static_assert(
        sqlpp::have_correct_cte_dependencies<sqlpp::dynamic_t<Basic>>());
    static_assert(sqlpp::have_correct_cte_dependencies<Basic, Recursive>());
    static_assert(
        sqlpp::have_correct_cte_dependencies<Basic,
                                             sqlpp::dynamic_t<Recursive>>());

    // Since `Recursive` does not reference `Basic`, they can be combined in any
    // order.
    static_assert(sqlpp::have_correct_cte_dependencies<Recursive, Basic>());
    static_assert(sqlpp::have_correct_cte_dependencies<Basic, Recursive>());
    static_assert(
        sqlpp::have_correct_cte_dependencies<sqlpp::dynamic_t<Recursive>,
                                             sqlpp::dynamic_t<Basic>>());
    static_assert(
        sqlpp::have_correct_cte_dependencies<sqlpp::dynamic_t<Basic>,
                                             sqlpp::dynamic_t<Recursive>>());
  }

  // Self-referencing CTEs can require other ctes. `have_correct_dependencies`
  // checks that, too.
  {
    auto basic = sqlpp::cte<"basic">().as(select(foo.id).from(foo));
    using Basic = decltype(basic);

    auto recursive_base =
        sqlpp::cte<"recursive">().as(select(basic.id.as<"a">()).from(basic));
    auto recursive =
        recursive_base.union_all(select((recursive_base.a + 1).as<"a">())
                                     .from(recursive_base)
                                     .where(recursive_base.a <= 10));
    using Recursive = decltype(recursive);

    // Simple good cases.
    static_assert(sqlpp::have_correct_cte_dependencies<Basic>());
    static_assert(
        sqlpp::have_correct_cte_dependencies<sqlpp::dynamic_t<Basic>>());
    static_assert(sqlpp::have_correct_cte_dependencies<Basic, Recursive>());
    static_assert(
        sqlpp::have_correct_cte_dependencies<Basic,
                                             sqlpp::dynamic_t<Recursive>>());
    static_assert(
        sqlpp::have_correct_cte_dependencies<sqlpp::dynamic_t<Basic>,
                                             sqlpp::dynamic_t<Recursive>>());

    // Since `Recursive` references `Basic`, the order matters
    static_assert(not sqlpp::have_correct_cte_dependencies<Recursive, Basic>());
    static_assert(
        not sqlpp::have_correct_cte_dependencies<sqlpp::dynamic_t<Recursive>,
                                                 sqlpp::dynamic_t<Basic>>());
  }

  // `with` exposes parameters from it's CTEs
  {
    auto a = sqlpp::parameter<"a", bool>();
    using A = decltype(a);
    auto b = sqlpp::parameter<"b", bool>();
    using B = decltype(b);

    auto basic_wp = sqlpp::cte<"basic">().as(select(foo.id).from(foo).where(a));
    auto referencing_wp = sqlpp::cte<"referencing">().as(
        select(basic_wp.id).from(basic_wp).where(b));

    {
      using W = extract_with_t<decltype(with(basic_wp))>;
      static_assert(std::is_same<sqlpp::parameters_of_t<W>,
                                 sqlpp::detail::type_vector<A>>::value);
    }
    {
      using W = extract_with_t<decltype(with(basic_wp, referencing_wp))>;
      static_assert(std::is_same<sqlpp::parameters_of_t<W>,
                                 sqlpp::detail::type_vector<A, B>>::value);
    }
  }
}

int main() {
  void test_with();
}
