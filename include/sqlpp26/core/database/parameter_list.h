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

#include <tuple>
#include <utility>

#include <sqlpp26/core/detail/type_vector.h>
#include <sqlpp26/core/indices.h>
#include <sqlpp26/core/query/bind_parameter.h>
#include <sqlpp26/core/type_traits.h>
#include <sqlpp26/core/wrong.h>

namespace sqlpp {
namespace detail {

template <typename... Parameters>
struct parameter_list_generator{
  struct parameters;
  consteval {
    std::vector<std::meta::info> parameter_data_members;
    template for (constexpr auto index : std::views::iota(size_t{}, sizeof...(Parameters))) {
      using Parameter = Parameters...[index];
      parameter_data_members.push_back(std::meta::data_member_spec(
          ^^typename Parameter::data_type,
          {.name = Parameter::name}));
    }
    define_aggregate(^^parameters, parameter_data_members);
  }
};

class result_row_bridge;
}  // namespace detail

template <typename T>
struct parameter_list_t {
  static_assert(
      wrong<parameter_list_t>,
      "Template parameter for parameter_list_t has to be a type_vector");
};

template <typename... Parameters>
struct parameter_list_t<detail::type_vector<Parameters...>>
    : public detail::parameter_list_generator<Parameters...>::parameters {

  parameter_list_t() = default;

  using _parameters = detail::parameter_list_generator<Parameters...>::parameters;

  template <typename Target>
  void _bind(Target& target) const {
    // Maybe unused if sizeof...(Parameters) == 0
    [[maybe_unused]] static constexpr auto [...Idx] = indices<sizeof...(Parameters)>;
    [[maybe_unused]] auto& [...parameters] = static_cast<const _parameters&>(*this);
    (..., bind_parameter(target, Idx, parameters));
  }
};

template <typename Exp>
using make_parameter_list_t = parameter_list_t<parameters_of_t<Exp>>;
}  // namespace sqlpp
