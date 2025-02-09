/*
 * Copyright (c) 2025, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sqlpp11/sqlpp11.h>
#include <sqlpp11/tests/core/constraints_helpers.h>
#include <sqlpp11/tests/core/tables.h>

namespace
{
  SQLPP_CREATE_NAME_TAG(something);

  // Returns true if `using_(declval<Expressions>()...)` is a valid function call.
  template <typename TypeVector, typename = void>
  struct can_call_using_with_impl : public std::false_type
  {
  };

  template <typename... Expressions>
  struct can_call_using_with_impl<sqlpp::detail::type_vector<Expressions...>,
                                  std::void_t<decltype(sqlpp::using_(std::declval<Expressions>()...))>>
      : public std::true_type
  {
  };

  template <typename... Expressions>
  struct can_call_using_with : public can_call_using_with_impl<sqlpp::detail::type_vector<Expressions...>>
  {
  };

}  // namespace

namespace test {
  SQLPP_CREATE_NAME_TAG(max_id);
}

int main()
{
  const auto maybe = true;
  const auto foo = test::TabFoo{};
  const auto bar = test::TabBar{};
  const auto c = cte(something).as(select(foo.id).from(foo).where(true));

  // using_(<non arguments>) is inconsistent and cannot be constructed.
  static_assert(not can_call_using_with<>::value, "");

  // using_(<non table>) cannot be called.
  static_assert(not can_call_using_with<decltype(foo.id)>::value, "");
  static_assert(not can_call_using_with<decltype(dynamic(true, foo.id))>::value, "");

  // using_(<table or join>) can be called, even if it is dynamic (we might not need the using).
  static_assert(can_call_using_with<decltype(foo)>::value, "");
  static_assert(can_call_using_with<decltype(foo.cross_join(bar))>::value, "");
  static_assert(can_call_using_with<decltype(c)>::value, "");

  static_assert(can_call_using_with<decltype(dynamic(maybe, foo))>::value, "");
  static_assert(can_call_using_with<decltype(dynamic(maybe, foo.cross_join(bar)))>::value, "");
  static_assert(can_call_using_with<decltype(dynamic(maybe, c))>::value, "");

  // using_ is not required
  {
    auto s = sqlpp::statement_t<sqlpp::no_using_t>{};
    using S = decltype(s);
    static_assert(std::is_same<sqlpp::statement_prepare_check_t<S>, sqlpp::consistent_t>::value, "");
  }

  // using_ must not require unknown ctes for prepare/run
  {
    auto s = delete_from(foo) << using_(c) << sqlpp::where(true);
    using S = decltype(s);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<S>, sqlpp::consistent_t>::value, "");
    static_assert(std::is_same<sqlpp::statement_prepare_check_t<S>, sqlpp::assert_no_unknown_ctes_t>::value, "");
  }

  // using_ must not repeat a table from from
  {
    auto s = delete_from(foo).using_(foo).where(true);
    using S = decltype(s);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<S>, sqlpp::consistent_t>::value, "");
    static_assert(
        std::is_same<sqlpp::statement_prepare_check_t<S>, sqlpp::assert_no_duplicate_table_providers_t>::value, "");
  }

}

