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
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
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

#include <sqlpp26/core/operator/sort_order_expression.h>

namespace sqlpp::filter {

template <typename Accessor>
struct sort_order_expression{
  template <typename Struct>
  constexpr auto& left(const Struct& l, const Struct& r) const {
    if (_sort_type == sort_type::asc) {
      return _accessor(l);
    }
    return _accessor(r);
  }

  template <typename Struct>
  constexpr auto& right(const Struct& l, const Struct& r) const {
    if (_sort_type == sort_type::asc) {
      return _accessor(r);
    }
    return _accessor(l);
  }

  template <typename Range>
  constexpr auto left_chunk(const Range& l, const Range& r) const {
    if (_sort_type == sort_type::asc) {
      return _accessor.aggregate(l);
    }
    return _accessor.aggregate(r);
  }

  template <typename Range>
  constexpr auto right_chunk(const Range& l, const Range& r) const {
    if (_sort_type == sort_type::asc) {
      return _accessor.aggregate(r);
    }
    return _accessor.aggregate(l);
  }

  Accessor _accessor;
  sort_type _sort_type;
};

}

namespace sqlpp {

template <typename Expr>
constexpr auto to_filter_expression(const sort_order_expression<Expr, sort_type>& expr) {
  return filter::sort_order_expression{to_filter_expression(read.lhs(expr)), read.rhs(expr)};
}

}  // namespace sqlpp

