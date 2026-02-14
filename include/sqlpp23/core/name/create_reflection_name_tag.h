#pragma once

/*
 * Copyright (c) 2026, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
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

#ifndef SQLPP_INCLUDE_REFLECTION
#if defined(__cpp_impl_reflection) && __cpp_impl_reflection >= 202506L
#define SQLPP_INCLUDE_REFLECTION 1
#else
#define SQLPP_INCLUDE_REFLECTION 0
#endif
#endif

#if SQLPP_INCLUDE_REFLECTION == 1

#include <meta>
#include <optional>

#include <sqlpp23/core/detail/fixed_string.h>

namespace sqlpp::meta {

template <::sqlpp::detail::fixed_string Alias>
struct reflection_alias {
  struct _sqlpp_name_tag {
    [[maybe_unused]] static constexpr bool require_quotes = false;
    [[maybe_unused]] static constexpr auto name = Alias;

    template <typename T>
    struct _member_impl_outer_t {
      struct _member_impl_inner_t;

      consteval {
        ::std::meta::define_aggregate(
            ^^_member_impl_inner_t,
            {
                ::std::meta::data_member_spec(
                    ^^T, {
                             .name = _sqlpp_name_tag::name,
                             .alignment = std::nullopt,
                             .bit_width = std::nullopt,
                             .no_unique_address = false})});
      }
    };

    // separate struct with base class necessary because it's not possible to
    // add a member function with `define_aggregate`
    template <typename T>
    struct _member_t : _member_impl_outer_t<T>::_member_impl_inner_t {
      auto& operator()(this auto&& self) {
        // used to access members of base
        constexpr auto ctx1 = ::std::meta::access_context::current();
        constexpr auto ctx2 = ctx1.via(^^_member_t);

        // get first (and single) member of this struct which has `Alias` as its
        // name
        return self
            .[: ::std::meta::nonstatic_data_members_of(
                    ^^typename _member_impl_outer_t<T>::_member_impl_inner_t,
                    ctx2)[0]:];
      }
    };
  };
};

template <::sqlpp::detail::fixed_string Alias>
consteval auto make_alias() -> ::sqlpp::meta::reflection_alias<Alias> {
  return {};
}

}  // namespace sqlpp::meta
#endif
