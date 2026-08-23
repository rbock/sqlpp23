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

#include <utility>

#include <sqlpp26/core/indices.h>
#include <sqlpp26/core/field_spec.h>
#include <sqlpp26/core/query/read_field.h>
#include <sqlpp26/core/query/bind_field.h>
#include <sqlpp26/core/query/result_row_fwd.h>

namespace sqlpp {
namespace detail {

template <typename... FieldSpecs>
struct result_row_generator{
  struct fields;
  consteval {
    std::vector<std::meta::info> field_data_members;
    template for (constexpr auto index : std::views::iota(size_t{}, sizeof...(FieldSpecs))) {
      using FieldSpec = FieldSpecs...[index];
      field_data_members.push_back(std::meta::data_member_spec(
          ^^typename FieldSpec::data_type,
          {.name = FieldSpec::name}));
    }
    define_aggregate(^^fields, field_data_members);
  }
};

class result_row_bridge;
}  // namespace detail

template <typename... FieldSpecs>
struct result_row_t
    : public detail::result_row_generator<FieldSpecs...>::fields {
  result_row_t() = default;

  result_row_t(const result_row_t&) = delete;
  result_row_t(result_row_t&&) = default;
  result_row_t& operator=(const result_row_t&) = delete;
  result_row_t& operator=(result_row_t&&) = default;

  bool operator==(const result_row_t& rhs) const {
    return _is_valid == rhs._is_valid;
  }

  explicit operator bool() const { return _is_valid; }

  friend auto as_tuple(const result_row_t& t) {
    const auto& [...fields] = static_cast<const _fields&>(t);
    return std::tie(fields...);
  }

 private:
  using _fields = detail::result_row_generator<FieldSpecs...>::fields;
  friend class detail::result_row_bridge;
  void _validate() { _is_valid = true; }

  void _invalidate() { _is_valid = false; }

  template <typename Target>
  void _bind_fields(Target& target) {
    static constexpr auto [...Idx] = indices<sizeof...(FieldSpecs)>;
    auto& [...fields] = static_cast<_fields&>(*this);
    (..., bind_field(target, Idx, fields));
  }

  template <typename Target>
  void _read_fields(Target& target) {
    static constexpr auto [...Idx] = indices<sizeof...(FieldSpecs)>;
    auto& [...fields] = static_cast<_fields&>(*this);
    (..., read_field(target, Idx, fields));
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
