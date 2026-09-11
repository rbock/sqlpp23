#pragma once

/*
 * Copyright (c) 2025, Roland Bock
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

#include <sqlpp26/core/type_traits.h>

namespace sqlpp {
namespace detail {
// Columns can be
// - Column
// - dynamic_t<Column>
// - as_expression_t<Column, NameTag>
// - dynamic_t<as_expression_t<Column, NameTag>>
//
// In order to analyse aggregate expressions, we want to ignore the
// as_expression aspects, but preserve the dynamic nature.
template <typename Column>
struct remove_as_from_select_column {
  using type = Column;
};
template <typename Column, typename NameTag>
struct remove_as_from_select_column<as_expression<Column, NameTag>> {
  using type = Column;
};
template <typename Column, typename NameTag>
struct remove_as_from_select_column<dynamic_t<as_expression<Column, NameTag>>> {
  using type = dynamic_t<Column>;
};

template <typename Column>
using remove_as_from_select_column_t =
    typename remove_as_from_select_column<Column>::type;

}  // namespace detail

}  // namespace sqlpp
