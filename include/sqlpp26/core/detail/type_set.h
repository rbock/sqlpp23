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

#include <sqlpp26/core/detail/type_info_set.h>

namespace sqlpp::detail {

// TODO: Would prefer to use flat_set::insert_range(), but that currently fails to compile
// See http://wg21.link/P3372
consteval void insert_type_info_set(auto& sink, const auto& data) {
  for (const auto& entry : data) {
    sink.insert(entry);
  }
}

consteval type_info_set make_joined_type_info_set(const auto&... Sets) {
  type_info_set all;

  (insert_type_info_set(all, Sets), ...);

  return all;
}

template <typename... T>
consteval type_info_set make_type_info_set() {
  type_info_set all;

  (all.insert(^^T), ...);

  return all;
}

template <typename... T>
struct are_unique {
  static constexpr bool value = (make_type_info_set<T...>().size() == sizeof...(T));
};

template <typename... T>
inline constexpr bool are_unique_v = are_unique<T...>::value;

}  // namespace sqlpp::detail
