#pragma once

/*
 * Copyright (c) 2024, Roland Bock
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

#include <type_traits>

namespace sqlpp {
struct no_name_t {
  static constexpr const char name[] = "_";
  template <typename T>
  struct _member_t {
    T _ = {};
    T& operator()() { return _; }
    const T& operator()() const { return _; }
  };
};

template <typename T>
concept HasOwnNameTag = requires {
  typename T::_sqlpp_name_tag;  // required nested name tag
};

template <typename T>
struct name_tag_of {
  using type = no_name_t;
};

template <typename T>
requires(HasOwnNameTag<T>)
struct name_tag_of<T> {
  using type = typename T::_sqlpp_name_tag;
};

template <typename T>
using name_tag_of_t = typename name_tag_of<T>::type;

template <typename T>
constexpr auto get_name_of(const T&) {
  return name_tag_of_t<T>::name;
}

// Override this for other classes like columns or tables.
template <typename T>
struct has_name_tag : public std::integral_constant<
                          bool,
                          not std::is_same_v<name_tag_of_t<T>, no_name_t>> {};

template <typename T>
inline constexpr bool has_name_tag_v = has_name_tag<T>::value;

}  // namespace sqlpp
