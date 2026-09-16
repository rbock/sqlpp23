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

#include <sqlpp26/mysql/database/serializer_context.h>
#include <sqlpp26/sqlpp26.h>

namespace sqlpp {

template <typename Lhs, typename Rhs, typename Condition>
constexpr void check_compatibility(type_v<mysql::context_t>,
                           type_v<join_t<Lhs, full_outer_join_t, Rhs, Condition>>) {
  throw std::domain_error("MySQL: No support for full outer join");
};

template <typename Expression, typename Type>
  requires(is_boolean<Expression>::value or std::is_same_v<Type, boolean>)
constexpr void check_compatibility(type_v<mysql::context_t>, type_v<cast_t<Expression, Type>>) {
  throw std::domain_error("MySQL: No support for bool cast");
};

template <typename L>
constexpr void check_compatibility(type_v<mysql::context_t>, type_v<sort_order_expression<L, sort_order_null>>) {
  throw std::domain_error("MySQL: No support for NULLS FIRST or NULLS LAST");
};

}  // namespace sqlpp
