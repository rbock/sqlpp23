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

#include <sqlpp26/core/chrono.h>
#include <sqlpp26/core/type_traits/optional.h>

namespace sqlpp {
  // TODO: This used to be undefined.
struct no_value_t{};

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

// TODO: Probably need to change this for insert and update?
template <typename T>
struct data_type_of<const T> {
  using type = data_type_of_t<T>;
};

template <typename T>
struct has_data_type
    : public std::integral_constant<
          bool,
          not std::is_same<data_type_of_t<T>, no_value_t>::value> {};

template <typename T>
inline constexpr bool has_data_type_v = has_data_type<T>::value;

template <typename T>
struct is_data_type : public std::false_type {};

template <typename T>
inline constexpr bool is_data_type_v = is_data_type<T>::value;

// boolean
template<typename T>
struct is_raw_boolean: public std::false_type {};

template<>
struct is_raw_boolean<bool>: public std::true_type {};

template <typename T>
requires(is_raw_boolean<T>::value)
struct is_data_type<T> : std::true_type {};

template <typename T>
  requires(is_raw_boolean<T>::value)
struct data_type_of<T> {
  using type = T;
};

template <typename T>
struct is_boolean
    : public is_raw_boolean<
          std::remove_const_t<remove_optional_t<data_type_of_t<T>>>> {};

template <typename T>
inline constexpr bool is_boolean_v = is_boolean<T>::value;

template <>
struct is_boolean<std::nullopt_t> : public std::true_type {};

// integral
template<typename T>
struct is_raw_integral: public std::false_type {};

template<>
struct is_raw_integral<int8_t>: public std::true_type {};

template<>
struct is_raw_integral<int16_t>: public std::true_type {};

template<>
struct is_raw_integral<int32_t>: public std::true_type {};

template<>
struct is_raw_integral<int64_t>: public std::true_type {};

template <typename T>
requires(is_raw_integral<T>::value)
struct is_data_type<T> : std::true_type {};

template <typename T>
  requires(is_raw_integral<T>::value)
struct data_type_of<T> {
  using type = T;
};

template <typename T>
struct is_integral
    : public is_raw_integral<
          std::remove_const_t<remove_optional_t<data_type_of_t<T>>>> {};

template <typename T>
inline constexpr bool is_integral_v = is_integral<T>::value;

template <>
struct is_integral<std::nullopt_t> : public std::true_type {};

// unsigned integral
template<typename T>
struct is_raw_unsigned_integral: public std::false_type {};

template<>
struct is_raw_unsigned_integral<uint8_t>: public std::true_type {};

template<>
struct is_raw_unsigned_integral<uint16_t>: public std::true_type {};

template<>
struct is_raw_unsigned_integral<uint32_t>: public std::true_type {};

template<>
struct is_raw_unsigned_integral<uint64_t>: public std::true_type {};

template <typename T>
requires(is_raw_unsigned_integral<T>::value)
struct is_data_type<T> : std::true_type {};

template <typename T>
  requires(is_raw_unsigned_integral<T>::value)
struct data_type_of<T> {
  using type = T;
};

template <typename T>
struct is_unsigned_integral
    : public is_raw_unsigned_integral<
          std::remove_const_t<remove_optional_t<data_type_of_t<T>>>> {};

template <typename T>
inline constexpr bool is_unsigned_integral_v = is_unsigned_integral<T>::value;

template <>
struct is_unsigned_integral<std::nullopt_t> : public std::true_type {};

// floating point
template<typename T>
struct is_raw_floating_point: public std::false_type {};

template<>
struct is_raw_floating_point<float>: public std::true_type {};

template<>
struct is_raw_floating_point<double>: public std::true_type {};

template<>
struct is_raw_floating_point<long double>: public std::true_type {};

template <typename T>
requires(is_raw_floating_point<T>::value)
struct is_data_type<T> : std::true_type {};

template <typename T>
  requires(is_raw_floating_point<T>::value)
struct data_type_of<T> {
  using type = T;
};

template <typename T>
struct is_floating_point
    : public is_raw_floating_point<
          std::remove_const_t<remove_optional_t<data_type_of_t<T>>>> {};

template <typename T>
inline constexpr bool is_floating_point_v = is_floating_point<T>::value;

template <>
struct is_floating_point<std::nullopt_t> : public std::true_type {};

// text
template<typename T>
struct is_raw_text: public std::false_type {};

template<>
struct is_raw_text<char>: public std::true_type {};

template<>
struct is_raw_text<const char*>: public std::true_type {};

template<>
struct is_raw_text<std::string>: public std::true_type {};

template<>
struct is_raw_text<std::string_view>: public std::true_type {};

template <typename T>
requires(is_raw_text<T>::value)
struct is_data_type<T> : std::true_type {};

template <typename T>
  requires(is_raw_text<T>::value)
struct data_type_of<T> {
  using type = T;
};

template <typename T>
struct is_text
    : public is_raw_text<
          std::remove_const_t<remove_optional_t<data_type_of_t<T>>>> {};

template <typename T>
inline constexpr bool is_text_v = is_text<T>::value;

template <>
struct is_text<std::nullopt_t> : public std::true_type {};

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
