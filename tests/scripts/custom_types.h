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

#include <sqlpp23/core/basic/table.h>
#include <sqlpp23/core/basic/table_columns.h>
#include <sqlpp23/core/name/create_name_tag.h>
#include <sqlpp23/core/type_traits.h>

namespace sample {
struct uuid {
  bool operator==(const uuid&) const = default;
};

template <typename T>
struct is_uuid : public std::is_same<sqlpp::remove_optional_t<sqlpp::data_type_of_t<T>>, uuid> {};

template <typename T>
inline constexpr bool is_uuid_v = is_uuid<T>::value;

template <typename Context>
auto to_sql_string(Context& context, const uuid& v) -> std::string {
  return "''";
}

template <typename Result>
void read_field(const Result& result, size_t index, uuid& v) {
}

template <typename Statement>
void bind_parameter(Statement& stmt, size_t index, const uuid& v) {
}

}  // namespace sample

namespace sqlpp {
template <>
struct data_type_of<sample::uuid> {
  using type = sample::uuid;
};

template <>
struct result_data_type_of<sample::uuid> {
  using type = sample::uuid;
};

template <>
struct parameter_value<sample::uuid> {
  using type = sample::uuid;
};

template <typename L, typename R>
  requires(sample::is_uuid_v<L> and sample::is_uuid_v<R>)
struct values_are_assignable<L, R> : public std::true_type {};

template <typename L, typename R>
  requires(sample::is_uuid_v<L> and sample::is_uuid_v<R>)
struct values_are_comparable<L, R> : public std::true_type {};

} // namespace sqlpp


