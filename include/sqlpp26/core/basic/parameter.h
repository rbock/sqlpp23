#pragma once

/*
 * Copyright (c) 2013-2015, Roland Bock
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

#include <sqlpp26/core/detail/type_set.h>
#include <sqlpp26/core/operator/enable_as.h>
#include <sqlpp26/core/operator/enable_comparison.h>
#include <sqlpp26/core/type_traits.h>

namespace sqlpp {
template <typename DataType, fixed_string Name>
struct parameter_t : public enable_as, public enable_comparison {
  static constexpr fixed_string name = Name;
  using data_type = DataType;
};

template <typename DataType, fixed_string Name>
struct parameters_of<parameter_t<DataType, Name>> {
  using type = detail::type_vector<parameter_t<DataType, Name>>;
};

template <typename DataType, fixed_string Name>
struct data_type_of<parameter_t<DataType, Name>> {
  using type = DataType;
};

template <typename Context, typename DataType, fixed_string Name>
auto to_sql_string(Context&, const parameter_t<DataType, Name>&)
    -> std::string {
  return "?";
}

template <fixed_string Name, typename DataType>
  requires(is_data_type_v<DataType>)
auto parameter()
    -> parameter_t<parameter_data_type_t<DataType>, Name> {
  return {};
}

template <typename T>
  requires(has_data_type_v<T> and has_name_v<T>)
auto parameter(const T&)
    -> parameter_t<parameter_data_type_t<data_type_of_t<T>>, name_of_v<T>> {
  return {};
}
}  // namespace sqlpp
