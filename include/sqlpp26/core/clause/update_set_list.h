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

#include <sqlpp26/core/detail/type_set.h>
#include <sqlpp26/core/query/dynamic.h>
#include <sqlpp26/core/query/statement.h>
#include <sqlpp26/core/reader.h>
#include <sqlpp26/core/tuple_to_sql_string.h>
#include <sqlpp26/core/type_traits.h>

namespace sqlpp {
template <typename... Assignments>
struct update_set_list_t {
  constexpr update_set_list_t(std::tuple<Assignments...> assignments)
      : _assignments(std::move(assignments)) {}
  update_set_list_t(const update_set_list_t&) = default;
  update_set_list_t(update_set_list_t&&) = default;
  update_set_list_t& operator=(const update_set_list_t&) = default;
  update_set_list_t& operator=(update_set_list_t&&) = default;
  ~update_set_list_t() = default;

 private:
  friend reader_t;
  std::tuple<Assignments...> _assignments;
};

template <typename Context, typename... Assignments>
auto to_sql_string(Context& context, const update_set_list_t<Assignments...>& t)
    -> std::string {
  return "SET " + tuple_to_sql_string(context, read.assignments(t),
                                       tuple_operand_no_dynamic{", "});
}

template <typename... Assignments>
struct is_clause<update_set_list_t<Assignments...>> : public std::true_type {};

template <typename Statement, typename... Assignments>
struct basic_consistency_check<Statement, update_set_list_t<Assignments...>> {
  static constexpr void verify() {
    using Clause = update_set_list_t<Assignments...>;
    if constexpr (not std::ranges::includes(Statement::get_provided_tables_of(),
                                            required_tables_of<Clause>::func(),
                                            sqlpp::detail::type_info_less{})) {
      throw std::domain_error(
          "at least one update assignment requires a table "
          "which is otherwise not known in the statement");
    }
    if constexpr (not std::ranges::includes(
                      Statement::get_static_provided_tables_of(),
                      required_static_tables_of<Clause>::func(),
                      sqlpp::detail::type_info_less{})) {
      throw std::domain_error(
          "at least one update assignment statically requires a table "
          "which is only known dynamically in the statement");
    }
  }
};

template <typename... Assignments>
struct nodes_of<update_set_list_t<Assignments...>> {
  using type = detail::type_vector<Assignments...>;
};

template <DynamicAssignment... Assignments>
inline constexpr bool are_valid_update_assignments =
    (sizeof...(Assignments) > 0 and
     // unique assignment columns
     detail::are_unique_v<
         lhs_t<remove_dynamic_t<Assignments>>...> and
     // assignment columns from exactly one table
     detail::make_joined_type_info_set(
         required_tables_of<lhs_t<Assignments>>::func()...).size() == 1);

struct no_update_set_list_t {
  template <typename Statement, DynamicAssignment... Assignments>
    requires(are_valid_update_assignments<Assignments...>)
  constexpr auto set(this Statement&& self, Assignments... assignments)

  {
    return new_statement<no_update_set_list_t>(
        std::forward<Statement>(self),
        update_set_list_t<Assignments...>{
            std::make_tuple(std::move(assignments)...)});
  }
};

template <typename Context>
auto to_sql_string(Context&, const no_update_set_list_t&) -> std::string {
  return "";
}

template <typename Statement>
struct basic_consistency_check<Statement, no_update_set_list_t> {
  static constexpr void verify() {
    throw std::domain_error("update assignments required, i.e. set(...)");
  }
};

template <DynamicAssignment... Assignments>
    requires(are_valid_update_assignments<Assignments...>)
constexpr auto update_set(Assignments... assignments) {
  return statement_t<no_update_set_list_t>().set(std::move(assignments)...);
}

}  // namespace sqlpp
