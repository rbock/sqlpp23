#pragma once

/*
 * Copyright (c) 2013-2016, Roland Bock
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

#include <utility>

#include <sqlpp23/core/field_spec.h>
#include <sqlpp23/core/query/result_row_fwd.h>

namespace sqlpp {
namespace detail {
template <typename IndexSequence, typename... FieldSpecs>
struct result_row_impl;

template <std::size_t index, typename FieldSpec>
struct result_field
    : public member_t<FieldSpec, typename FieldSpec::result_data_type> {
  using _field = member_t<FieldSpec, typename FieldSpec::result_data_type>;

 protected:
  result_field() = default;

  template <typename Target>
  void _bind_field(Target& target) {
    target.bind_field(index, _field::operator()());
  }

  template <typename Target>
  void _read_field(Target& target) {
    target.read_field(index, _field::operator()());
  }
};

template <std::size_t... Is, typename... FieldSpecs>
struct result_row_impl<std::index_sequence<Is...>, FieldSpecs...>
    : public result_field<Is, FieldSpecs>... {
 protected:
  result_row_impl() = default;

  template <typename Target>
  void _bind_fields(Target& target) {
    (result_field<Is, FieldSpecs>::_bind_field(target), ...);
  }

  template <typename Target>
  void _read_fields(Target& target) {
    (result_field<Is, FieldSpecs>::_read_field(target), ...);
  }

  auto _as_tuple() const {
    return std::tie(result_field<Is, FieldSpecs>::operator()()...);
  }

  static constexpr auto _get_sql_name_tuple() {
    return std::make_tuple(
        std::string_view{name_tag_of_t<FieldSpecs>::name}...);
  }
};

class result_row_bridge;
}  // namespace detail

template <typename... FieldSpecs>
struct result_row_t : public detail::result_row_impl<
                          std::make_index_sequence<sizeof...(FieldSpecs)>,
                          FieldSpecs...> {
  result_row_t() = default;

  result_row_t(const result_row_t&) = delete;
  result_row_t(result_row_t&&) = default;
  result_row_t& operator=(const result_row_t&) = delete;
  result_row_t& operator=(result_row_t&&) = default;

  // Can be removed in October 2026
  [[deprecated("use free function as_tuple(row) instead")]] auto as_tuple() const {
    return _impl::_as_tuple();
  }

  friend auto as_tuple(const result_row_t& row) {
    return row._as_tuple();
  }

  friend constexpr auto get_sql_name_tuple(const result_row_t&) {
    return _impl::_get_sql_name_tuple();
  }

  bool operator==(const result_row_t& rhs) const {
    return _is_valid == rhs._is_valid;
  }

  explicit operator bool() const { return _is_valid; }

 private:
  friend class detail::result_row_bridge;
  using _impl =
      detail::result_row_impl<std::make_index_sequence<sizeof...(FieldSpecs)>,
                              FieldSpecs...>;
  void _validate() { _is_valid = true; }

  void _invalidate() { _is_valid = false; }

  template <typename Target>
  void _bind_fields(Target& target) {
    _impl::_bind_fields(target);
  }

  template <typename Target>
  void _read_fields(Target& target) {
    _impl::_read_fields(target);
  }

  bool _is_valid{false};

};

namespace detail {
class result_row_bridge {
  public:
  template<typename... FieldSpecs, typename Target>
  void bind_fields(result_row_t<FieldSpecs...>& row, Target& target) {
    row._bind_fields(target);
  }

  template<typename... FieldSpecs, typename Target>
  void read_fields(result_row_t<FieldSpecs...>& row, Target& target) {
    row._read_fields(target);
  }

  template<typename... FieldSpecs>
  void validate(result_row_t<FieldSpecs...>& row) { row._validate(); }

  template<typename... FieldSpecs>
  void invalidate(result_row_t<FieldSpecs...>& row) { row._invalidate(); }
};
}  // namespace detail

template <typename Lhs, typename Rhs>
struct is_result_compatible {
  static constexpr auto value = false;
};

template <typename... LFields, typename... RFields>
  requires(sizeof...(LFields) == sizeof...(RFields))
struct is_result_compatible<result_row_t<LFields...>,
                            result_row_t<RFields...>> {
  static constexpr auto value =
      logic::all<is_field_compatible<LFields, RFields>::value...>::value;
};

}  // namespace sqlpp
