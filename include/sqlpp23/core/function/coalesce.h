#pragma once

/*
 * Copyright (c) 2025, Roland Bock
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

#include <sqlpp23/core/logic.h>
#include <sqlpp23/core/operator/enable_as.h>
#include <sqlpp23/core/operator/enable_comparison.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
template <typename... Args>
struct coalesce_t : public enable_comparison, public enable_as {
  coalesce_t(const Args... args) : _args(std::move(args)...) {}

  coalesce_t(const coalesce_t&) = default;
  coalesce_t(coalesce_t&&) = default;
  coalesce_t& operator=(const coalesce_t&) = default;
  coalesce_t& operator=(coalesce_t&&) = default;
  ~coalesce_t() = default;

  std::tuple<Args...> _args;
};

template <typename Arg, typename... Args>
struct data_type_of<coalesce_t<Arg, Args...>> {
  using type = std::conditional_t<
      logic::any<is_optional<data_type_of_t<Arg>>::value, is_optional<data_type_of_t<Args>>::value...>::value,
      force_optional_t<data_type_of_t<Arg>>,
      data_type_of_t<Arg>>;
};

template <typename... Args>
struct nodes_of<coalesce_t<Args...>> {
  using type = detail::type_vector<Args...>;
};

template <typename Context, typename... Args>
auto to_sql_string(Context& context, const coalesce_t<Args...>& t)
    -> std::string {
  return "COALESCE(" +
         tuple_to_sql_string(context, t._args, tuple_operand{", "}) + ")";
}

template <typename Arg, typename... Args>
  requires(
      has_data_type<remove_dynamic_t<Arg>>::value and
      logic::all<std::is_same_v<
          force_optional_t<data_type_of_t<remove_dynamic_t<Arg>>>,
          force_optional_t<data_type_of_t<remove_dynamic_t<Args>>>>...>::value)
auto coalesce(Arg arg, Args... args) -> coalesce_t<Arg, Args...> {
  return {std::move(arg), std::move(args)...};
}

}  // namespace sqlpp
