#pragma once

/**
 * Copyright © 2014-2019, Matthijs Möhlmann
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

#include <sqlpp23/core/concepts.h>
#include <sqlpp23/core/query/statement.h>
#include <sqlpp23/core/clause/on_conflict_do_nothing.h>
#include <sqlpp23/core/clause/on_conflict_do_update.h>
#include <sqlpp23/core/reader.h>

namespace sqlpp {

class assert_on_conflict_action_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(
        wrong<T...>,
        "either do_nothing() or do_update(...) is required with on_conflict");
  }
};

template <typename... Columns>
struct on_conflict_t {
  on_conflict_t(std::tuple<Columns...> columns)
      : _columns(std::move(columns)) {}
  on_conflict_t(const on_conflict_t&) = default;
  on_conflict_t(on_conflict_t&&) = default;
  on_conflict_t& operator=(const on_conflict_t&) = default;
  on_conflict_t& operator=(on_conflict_t&&) = default;
  ~on_conflict_t() = default;

  // DO NOTHING
  template <typename Statement>
  auto do_nothing(this Statement&& self) {
    auto new_clause = on_conflict_do_nothing_t<on_conflict_t>{self, true};
    return new_statement<on_conflict_t>(std::forward<Statement>(self),
                                        std::move(new_clause));
  }

  // DO UPDATE
  template <typename Statement, DynamicAssignment... Assignments>
    requires(
        sizeof...(Columns) > 0 and sizeof...(Assignments) > 0 and
        not sqlpp::detail::has_duplicates<
            typename lhs<Assignments>::type...>::value and
        sqlpp::detail::make_joined_set_t<
            required_tables_of_t<typename lhs<Assignments>::type>...>::size() ==
            1)
  auto do_update(this Statement&& self, Assignments... assignments) {
    auto new_clause = on_conflict_do_update_t<on_conflict_t, Assignments...>{
        self, std::make_tuple(std::move(assignments)...)};
    return new_statement<on_conflict_t>(std::forward<Statement>(self),
                                        std::move(new_clause));
  }

 private:
  friend ::sqlpp::reader_t;
  std::tuple<Columns...> _columns;
};

template <typename Context, typename... Columns>
auto to_sql_string(Context& context,
                   const on_conflict_t<Columns...>& t) -> std::string {
  const auto targets = tuple_to_sql_string(context, read.columns(t),
                                           tuple_operand_name_no_dynamic{", "});
  if (targets.empty()) {
    return " ON CONFLICT";
  }
  return " ON CONFLICT (" + targets + ")";
}

template <typename... Columns>
struct nodes_of<on_conflict_t<Columns...>> {
  using type = detail::type_vector<Columns...>;
};

template <typename Statement, typename... Columns>
struct consistency_check<Statement, on_conflict_t<Columns...>> {
  using type = assert_on_conflict_action_t;
  constexpr auto operator()() {
    return type{};
  }
};

struct no_on_conflict_t {
  template <typename Statement, DynamicColumn... Columns>
  auto on_conflict(this Statement&& self, Columns... columns) {
    return new_statement<no_on_conflict_t>(
        std::forward<Statement>(self),
        on_conflict_t<Columns...>{std::make_tuple(std::move(columns)...)});
  }
};

template<typename Context>
auto to_sql_string(Context&, const no_on_conflict_t&) -> std::string {
  return "";
}

template <typename Statement>
struct consistency_check<Statement, no_on_conflict_t> {
  using type = consistent_t;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename Expression>
struct is_clause<on_conflict_t<Expression>>
    : public std::true_type {};

template <DynamicColumn... Columns>
auto on_conflict(Columns... columns) {
  return statement_t<no_on_conflict_t>().on_conflict(std::move(columns)...);
}

}  // namespace sqlpp
