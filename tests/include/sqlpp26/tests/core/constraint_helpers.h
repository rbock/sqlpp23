#pragma once

/*
 * Copyright (c) 2024, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
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

#ifdef BUILD_WITH_MODULES
import sqlpp26.core;
#else
#include <sqlpp26/sqlpp26.h>
#endif

namespace detail {
template <typename S, sqlpp::fixed_string Expected>
consteval auto check_basic_consistency_message() -> std::string_view {
  std::string_view expected = Expected;
  try {
    S::check_basic_consistency();
    return std::define_static_string("missing expected exception");
  } catch (const std::domain_error& e) {
    if (e.what() != expected) {
      return std::define_static_string(std::format(
          "wrong exception message: '{}' != '{}'", expected, e.what()));
    }
    return {};
  }
}

template <typename S>
consteval auto check_no_basic_consistency_message() -> std::string_view {
  try {
    S::check_basic_consistency();
    return {};
  } catch (const std::domain_error& e) {
      return std::define_static_string(std::format(
          "unexpected exception: '{}'", e.what()));
  }
}
}

template <typename S, sqlpp::fixed_string Expected>
consteval bool expect_basic_consistency_fails() {
  constexpr auto message = detail::check_basic_consistency_message<S, Expected>();
  static_assert(message.empty(), message);
  return true;
}

template <typename S>
consteval bool expect_basic_consistency_succeeds() {
  constexpr auto message = detail::check_no_basic_consistency_message<S>();
  static_assert(message.empty(), message);
  return true;
}

namespace detail {
template <typename S, sqlpp::fixed_string Expected>
consteval auto check_prepare_consistency_message() -> std::string_view {
  std::string_view expected = Expected;
  try {
    S::check_prepare_consistency();
    return std::define_static_string("missing expected exception");
  } catch (const std::domain_error& e) {
    if (e.what() != expected) {
      return std::define_static_string(std::format(
          "wrong exception message: '{}' != '{}'", expected, e.what()));
    }
    return {};
  }
}

template <typename S>
consteval auto check_no_prepare_consistency_message() -> std::string_view {
  try {
    S::check_prepare_consistency();
    return {};
  } catch (const std::domain_error& e) {
      return std::define_static_string(std::format(
          "unexpected exception: '{}'", e.what()));
  }
}
}

template <typename S, sqlpp::fixed_string Expected>
consteval bool expect_prepare_consistency_fails() {
  constexpr auto message = detail::check_prepare_consistency_message<S, Expected>();
  static_assert(message.empty(), message);
  return true;
}

template <typename S>
consteval bool expect_prepare_consistency_succeeds() {
  constexpr auto message = detail::check_no_prepare_consistency_message<S>();
  static_assert(message.empty(), message);
  return true;
}

namespace detail {
template <sqlpp::fixed_string Expected>
consteval auto check_throws_message(auto&& callable) -> std::string_view {
  try {
    callable();
    return std::define_static_string("missing expected exception");
  } catch (const std::domain_error& e) {
    if (e.what() == std::string_view(Expected)) {
      return {};
    }
    return std::define_static_string(std::format(
        "wrong exception message: '{}' != '{}'", std::string_view(Expected), e.what()));
  }
}
}  // namespace detail

template <sqlpp::fixed_string Expected>
consteval bool expect_throws(auto&& callable) {
  constexpr auto message = detail::check_throws_message<Expected>(callable);
  static_assert(message.empty(), message);
  return true;
}



