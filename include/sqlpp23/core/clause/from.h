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

#include <sqlpp23/core/basic/table_ref.h>
#include <sqlpp23/core/concepts.h>
#include <sqlpp23/core/logic.h>
#include <sqlpp23/core/no_data.h>
#include <sqlpp23/core/query/statement.h>
#include <sqlpp23/core/reader.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
template <typename _Table>
struct from_t {
  from_t(_Table table) : _table(std::move(table)) {}

  from_t(const from_t&) = default;
  from_t(from_t&&) = default;
  from_t& operator=(const from_t&) = default;
  from_t& operator=(from_t&&) = default;
  ~from_t() = default;

 private:
  friend reader_t;
  _Table _table;
};

template <typename Context, typename _Table>
auto to_sql_string(Context& context, const from_t<_Table>& t) -> std::string {
  return dynamic_clause_to_sql_string(context, "FROM", read.table(t));
}

template <typename _Table>
struct is_clause<from_t<_Table>> : public std::true_type {};

template <typename Statement, typename _Table>
struct consistency_check<Statement, from_t<_Table>> {
  using type = consistent_t;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename _Table>
struct nodes_of<from_t<_Table>> {
  using type = detail::type_vector<_Table>;
};

template <typename _Table>
struct provided_tables_of<from_t<_Table>> : public provided_tables_of<_Table> {
};

template <typename _Table>
struct provided_static_tables_of<from_t<_Table>>
    : public provided_static_tables_of<_Table> {};

template <typename _Table>
struct provided_optional_tables_of<from_t<_Table>>
    : public provided_optional_tables_of<remove_dynamic_t<_Table>> {};

struct no_from_t {
  template <typename Statement, DynamicTable _Table>
  auto from(this Statement&& self, _Table table) {
    return new_statement<no_from_t>(
        std::forward<Statement>(self),
        from_t<table_ref_t<_Table>>{make_table_ref(table)});
  }
};

template <typename Context>
auto to_sql_string(Context&, const no_from_t&) -> std::string {
  return "";
}

template <typename Statement>
struct consistency_check<Statement, no_from_t> {
  using type = consistent_t;
  constexpr auto operator()() {
    return type{};
  }
};

template <DynamicTable T>
auto from(T t) {
  return statement_t<no_from_t>{}.from(std::move(t));
}

}  // namespace sqlpp
