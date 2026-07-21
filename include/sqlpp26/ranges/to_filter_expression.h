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

#include <array>
#include <optional>
#include <span>
#include <string>
#include <string_view>

#include <sqlpp26/core/chrono.h>
#include <sqlpp26/core/type_traits.h>
#include <sqlpp26/core/wrong.h>

namespace sqlpp {
template <typename T>
inline constexpr auto to_filter_expression(const T&) {
  static_assert(wrong<T>, "Missing specialization");
}

template <typename T>
struct filter_value {
  template <typename Struct>
  constexpr const auto& operator()(const Struct&) const {
    return value;
  }
  T value;
};

inline constexpr auto to_filter_expression(const bool& t) {
  return filter_value{t};
}

inline constexpr auto to_filter_expression(const int8_t& t) {
  return filter_value{t};
}

inline constexpr auto to_filter_expression(const int16_t& t) {
  return filter_value{t};
}

inline constexpr auto to_filter_expression(const int32_t& t) {
  return filter_value{t};
}

inline constexpr auto to_filter_expression(const int64_t& t) {
  return filter_value{t};
}

inline constexpr auto to_filter_expression(const uint8_t& t) {
  return filter_value{t};
}

inline constexpr auto to_filter_expression(const uint16_t& t) {
  return filter_value{t};
}

inline constexpr auto to_filter_expression(const uint32_t& t) {
  return filter_value{t};
}

inline constexpr auto to_filter_expression(const uint64_t& t) {
  return filter_value{t};
}

inline constexpr auto to_filter_expression(const float& t) {
  return filter_value{t};
}

inline constexpr auto to_filter_expression(const double& t) {
  return filter_value{t};
}

inline constexpr auto to_filter_expression(const long double& t) {
  return filter_value{t};
}

inline constexpr auto to_filter_expression(const std::string_view& t) {
  return filter_value{t};
}

inline constexpr auto to_filter_expression(const char* t) {
  return filter_value{t};
}

inline constexpr auto to_filter_expression(const std::string& t) {
  return filter_value{t};
}

inline constexpr auto to_filter_expression(const char& t) {
  return filter_value{t};
}

inline constexpr auto to_filter_expression(const std::span<const uint8_t>& t) {
  return filter_value{t};
}

inline constexpr auto to_filter_expression(const std::vector<uint8_t>& t) {
  return filter_value{t};
}

template <std::size_t N>
inline constexpr auto to_filter_expression(const std::array<uint8_t, N>& t) {
  return filter_value{t};
}

inline constexpr auto to_filter_expression(const std::chrono::sys_days& t) {
  return filter_value{t};
}

inline constexpr auto to_filter_expression(const std::chrono::microseconds& t) {
  return filter_value{t};
}

template <typename Period>
inline constexpr auto to_filter_expression(
    const std::chrono::time_point<std::chrono::system_clock, Period>& t) {
  return filter_value{t};
}

inline constexpr auto to_filter_expression(const std::nullopt_t&) {
  return filter_value{std::nullopt};
}

template <typename T>
inline constexpr auto to_filter_expression(const std::optional<T>& t) {
  if (not t.has_value()) {
    return to_filter_expression(std::nullopt);
  }
  return to_filter_expression(*t);
}

}  // namespace sqlpp
