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

#include <tuple>

#include <sqlpp23/core/clause/expression_static_check.h>
#include <sqlpp23/core/clause/simple_column.h>
#include <sqlpp23/core/concepts.h>
#include <sqlpp23/core/detail/type_set.h>
#include <sqlpp23/core/field_spec.h>
#include <sqlpp23/core/logic.h>
#include <sqlpp23/core/query/result_row.h>
#include <sqlpp23/core/query/statement.h>
#include <sqlpp23/core/reader.h>
#include <sqlpp23/core/tuple_to_sql_string.h>
#include <sqlpp23/core/type_traits.h>

// union_order_by is to be used with union, only.
// It is used to represent expressions like
//
// SELECT foo.id from foo
// UNION
// SELECT bar.id from bar
// ORDER BY id DESC; -- This must not be table qualified!
namespace sqlpp {

template <typename T>
struct sort_order_expression;

template <typename L>
struct simple_sort_order_expression {
  template<typename Column>
  constexpr simple_sort_order_expression(sort_order_expression<Column> s)
      : _lhs(read.lhs(s)), _rhs(read.rhs(s)) {}
  simple_sort_order_expression(const simple_sort_order_expression&) = default;
  simple_sort_order_expression(simple_sort_order_expression&&) = default;
  simple_sort_order_expression& operator=(const simple_sort_order_expression&) = default;
  simple_sort_order_expression& operator=(simple_sort_order_expression&&) = default;
  ~simple_sort_order_expression() = default;

 private:
  friend reader_t;
  simple_column_t<L> _lhs;
  sort_type _rhs;
};

template <typename Context, typename L>
auto to_sql_string(Context& context, const simple_sort_order_expression<L>& t)
    -> std::string {
  return operand_to_sql_string(context, read.lhs(t)) + to_sql_string(context, read.rhs(t));
}

namespace detail {
template <typename T>
struct sort_order_base {
  using type = void;
};
template <typename T>
struct sort_order_base<sort_order_expression<T>> {
  using type = T;
};
template <typename T>
struct sort_order_base<dynamic_t<sort_order_expression<T>>> {
  using type = T;
};
template <typename T>
using sort_order_base_t = typename sort_order_base<T>::type;

template <typename T>
struct simple_sort_order_base {
  using type = void;
};
template <typename T>
struct simple_sort_order_base<simple_sort_order_expression<T>> {
  using type = T;
};
template <typename T>
struct simple_sort_order_base<dynamic_t<simple_sort_order_expression<T>>> {
  using type = T;
};
template <typename T>
using simple_sort_order_base_t = typename simple_sort_order_base<T>::type;

template <typename T>
struct make_simple_sort_order {
  using type = void;
};
template <typename T>
struct make_simple_sort_order<sort_order_expression<T>> {
  using type = simple_sort_order_expression<T>;
};
template <typename T>
struct make_simple_sort_order<dynamic_t<sort_order_expression<T>>> {
  using type = dynamic_t<simple_sort_order_expression<T>>;
};
template <typename T>
using make_simple_sort_order_t = typename make_simple_sort_order<T>::type;
}

template <typename... Expressions>
struct union_order_by_t {
  union_order_by_t(Expressions... expressions) : _expressions(expressions...) {}

  union_order_by_t(const union_order_by_t&) = default;
  union_order_by_t(union_order_by_t&&) = default;
  union_order_by_t& operator=(const union_order_by_t&) = default;
  union_order_by_t& operator=(union_order_by_t&&) = default;
  ~union_order_by_t() = default;

 private:
  friend reader_t;
  std::tuple<Expressions...> _expressions;
};

template <typename Context, typename... Expressions>
auto to_sql_string(Context& context, const union_order_by_t<Expressions...>& t)
    -> std::string {
  return dynamic_tuple_clause_to_sql_string(context, "ORDER BY",
                                            read.expressions(t));
}

template <typename... Expressions>
struct is_clause<union_order_by_t<Expressions...>> : public std::true_type {};

template <typename... Expressions>
struct contains_order_by<union_order_by_t<Expressions...>> : public std::true_type {};

class assert_no_unknown_columns_in_union_sort_order_t
    : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>,
                  "at least one column in union order_by() does not match any "
                  "of the selected columns of the union");
  }
};

template <typename Lhs, typename RField>
struct is_column_in_result {
  static constexpr auto value = false;
};

template <typename... LFields, typename RField>
struct is_column_in_result<result_row_t<LFields...>, RField> {
  static constexpr auto value =
      logic::any<is_field_compatible<LFields, RField>::value...>::value;
};


template <typename Statement, typename... Expressions>
struct consistency_check<Statement, union_order_by_t<Expressions...>> {
  using type = static_check_t<
      logic::all<is_column_in_result<
          get_result_row_t<Statement>,
          make_field_spec_t<void, detail::simple_sort_order_base_t<Expressions>>>::value...>::value,
      assert_no_unknown_columns_in_union_sort_order_t>;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename Statement, typename... Expressions>
struct prepare_check<Statement, union_order_by_t<Expressions...>> {
  using type = consistent_t;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename... Expressions>
struct nodes_of<union_order_by_t<Expressions...>> {
  using type = detail::type_vector<Expressions...>;
};

// NO ORDER BY YET
struct no_union_order_by_t {
  template <typename Statement, DynamicSortOrder... Expressions>
    requires(sizeof...(Expressions) > 0 and
             (is_column_v<detail::sort_order_base_t<Expressions>> and ...))
  auto order_by(this Statement&& self, Expressions... expressions) {
    return new_statement<no_union_order_by_t>(
        std::forward<Statement>(self),
        union_order_by_t<detail::make_simple_sort_order_t<Expressions>...>{
            std::move(expressions)...});
  }
};

template <typename Context>
auto to_sql_string(Context&, const no_union_order_by_t&) -> std::string {
  return "";
}

template <typename Statement>
struct consistency_check<Statement, no_union_order_by_t> {
  using type = consistent_t;
  constexpr auto operator()() {
    return type{};
  }
};

template <DynamicSortOrder... Expressions>
  requires(sizeof...(Expressions) > 0 and
           (is_column_v<detail::sort_order_base_t<Expressions>> and ...))
auto union_order_by(Expressions... expressions) {
  return statement_t<no_union_order_by_t>().order_by(std::move(expressions)...);
}

}  // namespace sqlpp
