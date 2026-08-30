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

void test_required_tables_of() {
  // Columns require tables.
  {
    using T = decltype(test::tab_foo{}.id);
    static_assert(sqlpp::required_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<test::tab_foo>(),
                  "");
    static_assert(sqlpp::required_static_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<test::tab_foo>(),
                  "");
  }

  // Tables do not require tables.
  {
    using T = decltype(test::tab_foo{});
    static_assert(sqlpp::required_tables_of<T>::func().empty(), "");
    static_assert(sqlpp::required_static_tables_of<T>::func().empty(), "");
  }

  // Static expressions require collective tables.
  {
    using TF = test::tab_foo;
    using TB = test::tab_bar;
    using TC = decltype(test::tab_foo{}.as<"cheese">());
    using T = decltype(TF{}.id + TB{}.id + TC{}.id);
    static_assert(sqlpp::required_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<TF, TB, TC>(),
                  "");
    static_assert(sqlpp::required_static_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<TF, TB, TC>(),
                  "");
  }

  // Dynamic expressions require all tables, but on the the static parts
  // contribute to the statically required tables.
  {
    using TF = test::tab_foo;
    using TB = test::tab_bar;
    using TC = decltype(test::tab_foo{}.as<"cheese">());
    using T =
        decltype(TF{}.id < 17 and dynamic(true, TB{}.id < 17) and TC{}.id < 17);
    static_assert(sqlpp::required_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<TF, TB, TC>(),
                  "");
    static_assert(sqlpp::required_static_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<TF, TC>(),
                  "");
  }
}

void test_provided_tables_of() {
  // Columns do not provide tables.
  {
    using T = decltype(test::tab_foo{}.id);
    static_assert(sqlpp::provided_tables_of<T>::func().empty(), "");
    static_assert(sqlpp::provided_static_tables_of<T>::func().empty(), "");
    static_assert(sqlpp::provided_optional_tables_of<T>::func().empty(), "");
  }

  // Tables provide tables.
  {
    using T = test::tab_foo;
    static_assert(sqlpp::provided_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<T>(),
                  "");
    static_assert(sqlpp::provided_static_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<T>(),
                  "");
    static_assert(sqlpp::provided_optional_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<>(),
                  "");
  }

  // Tables AS provide tables.
  {
    using T = decltype(test::tab_foo{}.as<"cheese">());
    static_assert(sqlpp::provided_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<T>(),
                  "");
    static_assert(sqlpp::provided_static_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<T>(),
                  "");
    static_assert(sqlpp::provided_optional_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<>(),
                  "");
  }

  // SELECT AS provides tables.
  {
    using T =
        decltype(select(test::tab_foo{}.id).from(test::tab_foo{}).as<"cheese">());
    using Ref = sqlpp::select_ref_t<"cheese">;
    static_assert(sqlpp::provided_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<Ref>(),
                  "");
    static_assert(sqlpp::provided_static_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<Ref>(),
                  "");
    static_assert(sqlpp::provided_optional_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<>(),
                  "");
  }

  // JOIN provides tables.
  {
    using F = test::tab_foo;
    using B = test::tab_bar;
    using T = decltype(F{}.cross_join(B{}));
    static_assert(sqlpp::provided_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<F, B>(),
                  "");
    static_assert(sqlpp::provided_static_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<F, B>(),
                  "");
    static_assert(sqlpp::provided_optional_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<>(),
                  "");
  }

  // Dynamic JOIN provides non-static tables.
  {
    using F = test::tab_foo;
    using B = test::tab_bar;
    using T = decltype(F{}.cross_join(dynamic(true, B{})));
    static_assert(sqlpp::provided_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<F, B>(),
                  "");
    static_assert(sqlpp::provided_static_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<F>(),
                  "");
    static_assert(sqlpp::provided_optional_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<>(),
                  "");
  }

  // Left outer join makes right-hand-side table "optional", i.e. columns can by
  // NULL.
  {
    using F = test::tab_foo;
    using B = test::tab_bar;
    using T =
        decltype(F{}.left_outer_join(dynamic(true, B{})).on(F{}.id == B{}.id));
    static_assert(sqlpp::provided_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<F, B>(),
                  "");
    static_assert(sqlpp::provided_static_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<F>(),
                  "");
    static_assert(sqlpp::provided_optional_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<B>(),
                  "");
  }

  // Right outer join makes left-hand-side table "optional", i.e. columns can by
  // NULL.
  {
    using F = test::tab_foo;
    using B = test::tab_bar;
    using T =
        decltype(F{}.right_outer_join(dynamic(true, B{})).on(F{}.id == B{}.id));
    static_assert(sqlpp::provided_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<F, B>(),
                  "");
    static_assert(sqlpp::provided_static_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<F>(),
                  "");
    static_assert(sqlpp::provided_optional_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<F>(),
                  "");
  }

  // Full outer join makes left-hand-side table "optional", i.e. columns can by
  // NULL.
  {
    using F = test::tab_foo;
    using B = test::tab_bar;
    using T =
        decltype(F{}.full_outer_join(dynamic(true, B{})).on(F{}.id == B{}.id));
    static_assert(sqlpp::provided_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<F, B>(),
                  "");
    static_assert(sqlpp::provided_static_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<F>(),
                  "");
    static_assert(sqlpp::provided_optional_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<F, B>(),
                  "");
  }

  // Nested joins propagate their provided tables.
  {
    using F = test::tab_foo;
    using B = test::tab_bar;
    using C = decltype(test::tab_foo{}.as<"cheese">());
    using T = decltype(C{}.cross_join(
        F{}.full_outer_join(dynamic(true, B{})).on(F{}.id == B{}.id)));
    static_assert(sqlpp::provided_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<C, F, B>(),
                  "");
    static_assert(sqlpp::provided_static_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<C, F>(),
                  "");
    static_assert(sqlpp::provided_optional_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<F, B>(),
                  "");
  }

  // Nested joins propagate their provided tables.
  {
    using F = test::tab_foo;
    using B = test::tab_bar;
    using C = decltype(test::tab_foo{}.as<"cheese">());
    using T = decltype(F{}.full_outer_join(dynamic(true, B{}))
                           .on(F{}.id == B{}.id)
                           .cross_join(C{}));
    static_assert(sqlpp::provided_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<F, B, C>(),
                  "");
    static_assert(sqlpp::provided_static_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<F, C>(),
                  "");
    static_assert(sqlpp::provided_optional_tables_of<T>::func() ==
                               sqlpp::detail::make_type_info_set<F, B>(),
                  "");
  }
}

int main() {
  void test_required_tables_of();
  void test_provided_tables_of();
}
