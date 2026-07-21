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

#include <array>
#include <chrono>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>

#include <sqlpp23/core/chrono.h>
#include <sqlpp23/core/type_traits/optional.h>

namespace sqlpp {
struct no_value_t;

template <typename T>
struct data_type_of {
  using type = no_value_t;
};

template <typename T>
using data_type_of_t = typename data_type_of<T>::type;

template <typename T>
struct data_type_of<std::optional<T>> {
  using type = force_optional_t<data_type_of_t<T>>;
};

template <typename T>
struct has_data_type
    : public std::integral_constant<
          bool,
          not std::is_same<data_type_of_t<T>, no_value_t>::value> {};

template <typename T>
struct is_data_type : public std::false_type {};

template <typename T>
inline constexpr bool has_data_type_v = has_data_type<T>::value;

struct boolean {};

template<>
struct is_data_type<boolean> : std::true_type {};

template <>
struct data_type_of<bool> {
  using type = boolean;
};

struct integral {};
template <>
struct is_data_type<integral> : std::true_type {};

template <>
struct data_type_of<int8_t> {
  using type = integral;
};
template <>
struct data_type_of<int16_t> {
  using type = integral;
};
template <>
struct data_type_of<int32_t> {
  using type = integral;
};
template <>
struct data_type_of<int64_t> {
  using type = integral;
};

struct unsigned_integral {};
template <>
struct is_data_type<unsigned_integral> : std::true_type {};

template <>
struct data_type_of<uint8_t> {
  using type = unsigned_integral;
};
template <>
struct data_type_of<uint16_t> {
  using type = unsigned_integral;
};
template <>
struct data_type_of<uint32_t> {
  using type = unsigned_integral;
};
template <>
struct data_type_of<uint64_t> {
  using type = unsigned_integral;
};

struct floating_point {};
template <>
struct is_data_type<floating_point> : std::true_type {};

template <>
struct data_type_of<float> {
  using type = floating_point;
};
template <>
struct data_type_of<double> {
  using type = floating_point;
};
template <>
struct data_type_of<long double> {
  using type = floating_point;
};

struct text {};
template <>
struct is_data_type<text> : std::true_type {};

template <>
struct data_type_of<char> {
  using type = text;
};
template <>
struct data_type_of<const char*> {
  using type = text;
};
template <>
struct data_type_of<std::string> {
  using type = text;
};
template <>
struct data_type_of<std::string_view> {
  using type = text;
};

struct blob {};
template <>
struct is_data_type<blob> : std::true_type {};

template <std::size_t N>
struct data_type_of<std::array<std::uint8_t, N>> {
  using type = blob;
};
template <>
struct data_type_of<std::vector<std::uint8_t>> {
  using type = blob;
};
template <>
struct data_type_of<std::span<std::uint8_t>> {
  using type = blob;
};

// date
struct date {};
template <>
struct is_data_type<date> : std::true_type {};

template <>
struct data_type_of<std::chrono::sys_days> {
  using type = date;
};

// time of day
struct time {};
template <>
struct is_data_type<time> : std::true_type {};

template <typename Rep, typename Period>
struct data_type_of<std::chrono::duration<Rep, Period>> {
  using type = time;
};

// timestamp aka date_time
struct timestamp {};
template <>
struct is_data_type<timestamp> : std::true_type {};

template <typename Period>
requires(Period{1} < std::chrono::days{1})
struct data_type_of<
    std::chrono::time_point<std::chrono::system_clock, Period>> {
  using type = timestamp;
};

template <typename T>
struct is_boolean
    : public std::is_same<remove_optional_t<data_type_of_t<T>>, boolean> {};

template <typename T>
inline constexpr bool is_boolean_v = is_boolean<T>::value;

template <>
struct is_boolean<std::nullopt_t> : public std::true_type {};

template <typename T>
struct is_integral
    : public std::is_same<remove_optional_t<data_type_of_t<T>>, integral> {};

template <>
struct is_integral<std::nullopt_t> : public std::true_type {};

template <typename T>
struct is_unsigned_integral
    : public std::is_same<remove_optional_t<data_type_of_t<T>>,
                          unsigned_integral> {};

template <>
struct is_unsigned_integral<std::nullopt_t> : public std::true_type {};

template <typename T>
struct is_floating_point
    : public std::is_same<remove_optional_t<data_type_of_t<T>>,
                          floating_point> {};

template <>
struct is_floating_point<std::nullopt_t> : public std::true_type {};

// A generic numeric type which could be (unsigned) integral or floating point.
struct numeric {};
template <typename T>
struct is_numeric
    : public std::integral_constant<
          bool,
          is_boolean<T>::value or is_integral<T>::value or
              is_unsigned_integral<T>::value or is_floating_point<T>::value or
              std::is_same<remove_optional_t<data_type_of_t<T>>,
                           numeric>::value> {};

template <>
struct is_numeric<std::nullopt_t> : public std::true_type {};

template <typename T>
struct is_text
    : public std::is_same<remove_optional_t<data_type_of_t<T>>, text> {};

template <>
struct is_text<std::nullopt_t> : public std::true_type {};

template <typename T>
struct is_blob
    : public std::is_same<remove_optional_t<data_type_of_t<T>>, blob> {};

template <>
struct is_blob<std::nullopt_t> : public std::true_type {};

template <typename T>
struct is_date
    : public std::is_same<remove_optional_t<data_type_of_t<T>>, date> {};

template <>
struct is_date<std::nullopt_t> : public std::true_type {};

template <typename T>
struct is_timestamp
    : public std::is_same<remove_optional_t<data_type_of_t<T>>, timestamp> {};

template <>
struct is_timestamp<std::nullopt_t> : public std::true_type {};

template <typename T>
struct is_date_or_timestamp
    : public std::integral_constant<bool,
                                    is_date<T>::value or
                                        is_timestamp<T>::value> {};

template <typename T>
struct is_time
    : public std::is_same<remove_optional_t<data_type_of_t<T>>, time> {
};

template <>
struct is_time<std::nullopt_t> : public std::true_type {};

}  // namespace sqlpp
