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

#include <sqlpp23/core/basic/join_fwd.h>
#include <sqlpp23/core/type_traits.h>
#include <utility>

namespace sqlpp {
struct enable_join {
 public:
  template <typename Table, typename T>
  auto join(this Table&& self, T t) {
    return ::sqlpp::join(std::forward<Table>(self), std::move(t));
  }

  template <typename Table, typename T>
  auto inner_join(this Table&& self, T t) {
    return ::sqlpp::inner_join(std::forward<Table>(self), std::move(t));
  }

  template <typename Table, typename T>
  auto left_outer_join(this Table&& self, T t) {
    return ::sqlpp::left_outer_join(std::forward<Table>(self), std::move(t));
  }

  template <typename Table, typename T>
  auto right_outer_join(this Table&& self, T t) {
    return ::sqlpp::right_outer_join(std::forward<Table>(self), std::move(t));
  }

  template <typename Table, typename T>
  auto full_outer_join(this Table&& self, T t) {
    return ::sqlpp::full_outer_join(std::forward<Table>(self), std::move(t));
  }

  template <typename Table, typename T>
  auto cross_join(this Table&& self, T t) {
    return ::sqlpp::cross_join(std::forward<Table>(self), std::move(t));
  }
};

template <typename T>
struct has_enabled_join : public std::is_base_of<enable_join, T> {};

}  // namespace sqlpp
