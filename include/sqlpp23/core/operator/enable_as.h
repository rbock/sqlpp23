#pragma once

/*
Copyright (c) 2024, Roland Bock
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <sqlpp23/core/operator/as_expression.h>
#include <sqlpp23/core/type_traits.h>

#include <sqlpp23/core/name/create_reflection_name_tag.h>

namespace sqlpp {
// To be used as CRTP base for expressions that should offer the as() member
// function.
class enable_as {
 public:
  template <typename Expr, typename NameTagProvider>
  constexpr auto as(this Expr&& self, const NameTagProvider& alias)
      -> decltype(::sqlpp::as(std::forward<Expr>(self), alias)) {
    return ::sqlpp::as(std::forward<Expr>(self), alias);
  }

#if SQLPP_INCLUDE_REFLECTION == 1
  template <::sqlpp::detail::fixed_string Alias, typename Expr>
  constexpr auto as(this Expr&& self)
      -> decltype(::sqlpp::as(::std::forward<Expr>(self),
                              ::sqlpp::meta::make_alias<Alias>())) {
    return ::sqlpp::as(::std::forward<Expr>(self),
                       ::sqlpp::meta::make_alias<Alias>());
  }
#endif
};

template <typename T>
struct has_enabled_as : public std::is_base_of<enable_as, T> {};

}  // namespace sqlpp
