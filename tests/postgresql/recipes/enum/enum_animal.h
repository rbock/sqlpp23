#pragma once

/*
 * Copyright (c) 2026, Vesselin Atanasov
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

// The actual SQL representation of our C++ enum that will be stored in a
// nullable integer column
enum class animal { bird, cat, dog, fish };

// Helper metafunctions that check if a variable is of our (possibly optional)
// enum type
template <typename T>
struct is_animal
    : public std::is_same<sqlpp::remove_optional_t<sqlpp::data_type_of_t<T>>,
                          animal> {};
template <typename T>
static constexpr bool is_animal_v = is_animal<T>::value;

// Required to enable serialization of the enum
template <typename Context>
auto to_sql_string(Context& context, const animal& value) {
  return sqlpp::to_sql_string(context, static_cast<int64_t>(value));
}

// Required to enable reading of the enum from the database
template <typename Result>
void read_field(const Result& result, size_t field_index, animal& value) {
  int64_t iv{};
  read_field(result, field_index, iv);
  value = static_cast<animal>(iv);
}

// Only needed if you want to use the enum as a paratemer in prepared statements
template <typename Statement>
void bind_parameter(Statement& stmt, size_t index, const animal& value) {
  bind_parameter(stmt, index, static_cast<int64_t>(value));
}

namespace sqlpp {

// Required to enable the library to calculate the data type of any expression
// that uses our enum
template <>
struct data_type_of<animal> {
  using type = animal;
};

// Required to enable reading and deserialization of the enum from the database
template <>
struct result_data_type_of<animal> {
  using type = animal;
};

// Required to enable enum assignment during INSERT queries
template <typename L, typename R>
  requires(is_animal_v<L> && is_animal_v<R>)
struct values_are_assignable<L, R> : public std::true_type {};

// Only needed if you want to use the enum in ORDER BY clauses or compare it
// using ==, <, etc.
template <typename L, typename R>
  requires(is_animal_v<L> && is_animal_v<R>)
struct values_are_comparable<L, R> : public std::true_type {};

}  // namespace sqlpp
