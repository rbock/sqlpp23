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

#include <sqlpp23/core/operator/comparison_functions.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
// To be used as CRTP base for expressions that should offer the comparison
// member functions. This also enables sort order member functions
class enable_comparison {
 public:
  template <typename Expr, typename... Args>
  constexpr auto in(this Expr&& self, std::tuple<Args...> args)
      -> decltype(::sqlpp::in(std::forward<Expr>(self), std::move(args))) {
    return ::sqlpp::in(std::forward<Expr>(self), std::move(args));
  }

  template <typename Expr, typename... Args>
  constexpr auto in(this Expr&& self, Args... args)
      -> decltype(::sqlpp::in(std::forward<Expr>(self), std::move(args)...)) {
    return ::sqlpp::in(std::forward<Expr>(self), std::move(args)...);
  }

  template <typename Expr, typename Arg>
  constexpr auto in(this Expr&& self, std::vector<Arg> args)
      -> decltype(::sqlpp::in(std::forward<Expr>(self), std::move(args))) {
    return ::sqlpp::in(std::forward<Expr>(self), std::move(args));
  }

  template <typename Expr, typename... Args>
  constexpr auto not_in(this Expr&& self, std::tuple<Args...> args)
      -> decltype(::sqlpp::not_in(std::forward<Expr>(self), std::move(args))) {
    return ::sqlpp::not_in(std::forward<Expr>(self), std::move(args));
  }

  template <typename Expr, typename... Args>
  constexpr auto not_in(this Expr&& self, Args... args)
      -> decltype(::sqlpp::not_in(std::forward<Expr>(self),
                                  std::move(args)...)) {
    return ::sqlpp::not_in(std::forward<Expr>(self), std::move(args)...);
  }

  template <typename Expr, typename Arg>
  constexpr auto not_in(this Expr&& self, std::vector<Arg> args)
      -> decltype(::sqlpp::not_in(std::forward<Expr>(self), std::move(args))) {
    return ::sqlpp::not_in(std::forward<Expr>(self), std::move(args));
  }

  template <typename Expr>
  constexpr auto is_null(this Expr&& self)
      -> decltype(::sqlpp::is_null(std::forward<Expr>(self))) {
    return ::sqlpp::is_null(std::forward<Expr>(self));
  }

  template <typename Expr>
  constexpr auto is_not_null(this Expr&& self)
      -> decltype(::sqlpp::is_not_null(std::forward<Expr>(self))) {
    return ::sqlpp::is_not_null(std::forward<Expr>(self));
  }

  template <typename Expr, typename R>
  constexpr auto is_distinct_from(this Expr&& self, R r)
      -> decltype(::sqlpp::is_distinct_from(std::forward<Expr>(self),
                                            std::move(r))) {
    return ::sqlpp::is_distinct_from(std::forward<Expr>(self), std::move(r));
  }

  template <typename Expr, typename R>
  constexpr auto is_not_distinct_from(this Expr&& self, R r)
      -> decltype(::sqlpp::is_not_distinct_from(std::forward<Expr>(self),
                                                std::move(r))) {
    return ::sqlpp::is_not_distinct_from(std::forward<Expr>(self),
                                         std::move(r));
  }

  template <typename Expr, typename R1, typename R2>
  constexpr auto between(this Expr&& self, R1 r1, R2 r2)
      -> decltype(::sqlpp::between(std::forward<Expr>(self),
                                   std::move(r1),
                                   std::move(r2))) {
    return ::sqlpp::between(std::forward<Expr>(self), std::move(r1),
                            std::move(r2));
  }

  template <typename Expr>
  constexpr auto asc(this Expr&& self)
      -> decltype(::sqlpp::asc(std::forward<Expr>(self))) {
    return ::sqlpp::asc(std::forward<Expr>(self));
  }

  template <typename Expr>
  constexpr auto desc(this Expr&& self)
      -> decltype(::sqlpp::desc(std::forward<Expr>(self))) {
    return ::sqlpp::desc(std::forward<Expr>(self));
  }

  template <typename Expr>
  constexpr auto order(this Expr&& self, ::sqlpp::sort_type t)
      -> decltype(::sqlpp::order(std::forward<Expr>(self), t)) {
    return ::sqlpp::order(std::forward<Expr>(self), t);
  }

  template <typename Expr, typename R>
  constexpr auto like(this Expr&& self, R r)
      -> decltype(::sqlpp::like(std::forward<Expr>(self), std::move(r))) {
    return ::sqlpp::like(std::forward<Expr>(self), std::move(r));
  }
};

template <typename T>
struct has_enabled_comparison : public std::is_base_of<enable_comparison, T> {};

}  // namespace sqlpp
