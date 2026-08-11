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

#include <sqlpp26/core/basic/enable_join.h>
#include <sqlpp26/core/basic/table_ref.h>
#include <sqlpp26/core/basic/field_column.h>
#include <sqlpp26/core/clause/select_flags.h>
#include <sqlpp26/core/logic.h>
#include <sqlpp26/core/query/result_row.h>
#include <sqlpp26/core/query/statement_fwd.h>
#include <sqlpp26/core/reader.h>
#include <sqlpp26/core/tuple_to_sql_string.h>
#include <sqlpp26/core/type_traits.h>

namespace sqlpp {
  // TODO
#if 0
template <typename Flag, typename Lhs, typename Rhs>
struct cte_union_t {
  cte_union_t(Lhs lhs, Rhs rhs) : _lhs(std::move(lhs)), _rhs(std::move(rhs)) {}

  cte_union_t(const cte_union_t&) = default;
  cte_union_t(cte_union_t&&) = default;
  cte_union_t& operator=(const cte_union_t&) = default;
  cte_union_t& operator=(cte_union_t&&) = default;
  ~cte_union_t() = default;

 private:
  friend reader_t;
  Lhs _lhs;
  Rhs _rhs;
};

template <typename Context, typename Flag, typename Lhs, typename Rhs>
auto to_sql_string(Context& context, const cte_union_t<Flag, Lhs, Rhs>& t)
    -> std::string {
  if constexpr (is_dynamic<Rhs>::value) {
    if (read.rhs(t).has_value()) {
      return to_sql_string(context, read.lhs(t)) + " UNION " +
             to_sql_string(context, Flag{}) +
             to_sql_string(context, read.rhs(t).value());
    }
    return to_sql_string(context, read.lhs(t));
  } else {
    return to_sql_string(context, read.lhs(t)) + " UNION " +
           to_sql_string(context, Flag{}) + to_sql_string(context, read.rhs(t));
  }
}

template <typename Flag, typename Lhs, typename Rhs>
struct nodes_of<cte_union_t<Flag, Lhs, Rhs>> {
  using type = detail::type_vector<Lhs, Rhs>;
};
#endif

template <fixed_string Name, typename Statement>
struct cte_t;

template <fixed_string Name>
struct cte_ref_t;

template <fixed_string Name, typename Statement, typename... FieldSpecs>
struct table_ref<cte_t<Name, Statement, FieldSpecs...>> {
  using type = cte_ref_t<Name>;
};

template <fixed_string Name, typename Statement, typename... FieldSpecs>
struct table_ref<dynamic_t<cte_t<Name, Statement, FieldSpecs...>>> {
  using type = dynamic_t<cte_ref_t<Name>>;
};

template <fixed_string Name, typename Statement, typename... FieldSpecs>
auto make_table_ref(
    cte_t<Name, Statement, FieldSpecs...> /* unused */)
    -> cte_ref_t<Name> {
  return {};
}

template <fixed_string Name, typename Statement, typename... FieldSpecs>
auto make_table_ref(
    dynamic_t<cte_t<Name, Statement, FieldSpecs...>> dyn_cte)
    -> dynamic_t<cte_ref_t<Name>> {
  if (dyn_cte.has_value()) {
    return {cte_ref_t<Name>{}};
  }
  return {std::nullopt};
}

template <fixed_string Name, typename Select, typename ResultRow>
struct cte_generator;

template <fixed_string Name, typename Select, typename... FieldSpecs>
struct cte_generator<Name, Select, result_row_t<FieldSpecs...>>{
  struct columns;
  consteval {
    std::vector<std::meta::info> column_data_members;
    std::array field_specs = {^^FieldSpecs...};
    template for (constexpr auto index : std::views::iota(size_t{}, sizeof...(FieldSpecs))) {
      using FieldSpec = FieldSpecs...[index];
      column_data_members.push_back(std::meta::data_member_spec(
          substitute(^^sqlpp::field_column, {^^cte_ref_t<Name>, field_specs[index]}),
          {.name = FieldSpec::name}));
    }
    define_aggregate(^^columns, column_data_members);
  }
  template<fixed_string Alias>
  struct cte_as_columns {
    struct type;
    consteval {
      std::vector<std::meta::info> column_data_members;
      std::array field_specs = {^^FieldSpecs...};
      template for (constexpr auto index : std::views::iota(size_t{}, sizeof...(FieldSpecs))) {
        using FieldSpec = FieldSpecs...[index];
        column_data_members.push_back(std::meta::data_member_spec(
              // TODO: Maybe introduce cte_as_ref to avoid simple accidents?
            substitute(^^sqlpp::field_column, {^^cte_ref_t<Alias>, field_specs[index]}),
            {.name = FieldSpec::name}));
      }
      define_aggregate(^^type, column_data_members);
    }
  };
};


template <fixed_string Name, typename Statement,
          fixed_string Alias>
struct cte_as_t : public cte_generator<Name, Statement, get_result_row_t<Statement>>::cte_as_columns<Alias>::type,
                  public enable_join {
  template <typename Context>
  friend auto to_sql_string(Context& context, const cte_as_t&) -> std::string {
    return name_to_sql_string(context, Name) +
           " AS " +
           name_to_sql_string(context, Alias);
  }
};

template <fixed_string Name, typename Statement, fixed_string Alias>
struct is_table<cte_as_t<Name, Statement, Alias>> : public std::true_type {};

template <fixed_string Name, typename Statement, fixed_string Alias>
struct name_of<cte_as_t<Name, Statement, Alias>> {
  static constexpr fixed_string value = Alias;
};

template <fixed_string Name, typename Statement, fixed_string Alias>
struct provided_tables_of<cte_as_t<Name, Statement, Alias>> {
  static consteval auto func() -> detail::type_info_set {
    return detail::make_type_info_set<cte_ref_t<Alias>>();
  }
};

template <fixed_string Name, typename Statement, fixed_string Alias>
struct required_ctes_of<cte_as_t<Name, Statement, Alias>> {
  // An aliased CTE requires the original CTE from the WITH clause.
  static consteval auto func() -> detail::type_info_set {
    return detail::make_type_info_set<cte_ref_t<Name>>();
  }
};

template <fixed_string Name, typename Statement, fixed_string Alias>
struct required_static_ctes_of<cte_as_t<Name, Statement, Alias>>
    : public required_ctes_of<cte_as_t<Name, Statement, Alias>> {};

#if 0
template <typename Lhs, typename Rhs>
inline constexpr bool are_valid_cte_union_args =
    (is_statement<Lhs>::value and is_statement<Rhs>::value and
     required_tables_of_t<Lhs>::empty() and
     required_tables_of_t<Rhs>::empty() and
     has_result_row<Lhs>::value and has_result_row<Rhs>::value and
     is_result_compatible<get_result_row_t<Lhs>, get_result_row_t<Rhs>>::value);

template <typename Lhs, typename Rhs>
inline constexpr bool are_valid_cte_union_args<Lhs, dynamic_t<Rhs>> =
    are_valid_cte_union_args<Lhs, Rhs>;
#endif

template <fixed_string Name, typename Statement, fixed_string Alias>
constexpr auto all_of(const cte_as_t<Name, Statement, Alias>&) {
  const auto& [...columns] = typename cte_generator<Name, Statement, get_result_row_t<Statement>>::cte_as_columns<Alias>::type{};

  return std::make_tuple(columns...);
}

template <fixed_string Name, typename Statement>
struct cte_t
    : public cte_generator<Name, Statement, get_result_row_t<Statement>>::columns,
      public enable_join {
        /*
  using _column_tuple_t =
      std::tuple<column_t<cte_ref_t<Name>, FieldSpecs>...>;
      */

        /*
  using _result_row_t = result_row_t<FieldSpecs...>;
  */

  constexpr cte_t(Statement statement) : _expression(std::move(statement)) {}
  cte_t(const cte_t&) = default;
  cte_t(cte_t&&) = default;
  cte_t& operator=(const cte_t&) = default;
  cte_t& operator=(cte_t&&) = default;
  ~cte_t() = default;

  template <fixed_string Alias>
  constexpr auto as() const
      -> cte_as_t<Name, Statement, Alias> {
    return {};
  }

#if 0
  template <typename Rhs>
    requires(are_valid_cte_union_args<Statement, Rhs>)
  auto union_distinct(Rhs rhs) const
      -> cte_t<Name,
               cte_union_t<distinct_t, Statement, Rhs>,
               FieldSpecs...> {
    return cte_union_t<distinct_t, Statement, Rhs>{_expression, rhs};
  }

  template <typename Rhs>
    requires(are_valid_cte_union_args<Statement, Rhs>)
  auto union_all(Rhs rhs) const -> cte_t<Name,
                                         cte_union_t<all_t, Statement, Rhs>,
                                         FieldSpecs...> {
    return cte_union_t<all_t, Statement, Rhs>{_expression, rhs};
  }
#endif

 private:
  friend reader_t;
  Statement _expression;
};

template <fixed_string Name, typename Statement>
constexpr auto all_of(const cte_t<Name, Statement>&) {
  const auto& [...columns] = typename cte_generator<Name, Statement, get_result_row_t<Statement>>::columns{};

  return std::make_tuple(columns...);
}

template <typename Context,
          fixed_string Name,
          typename Statement,
          typename... ColumnSpecs>
auto to_sql_string(Context& context,
                   const cte_t<Name, Statement, ColumnSpecs...>& t)
    -> std::string {
  return name_to_sql_string(context, Name) +
         " AS (" + to_sql_string(context, read.expression(t)) + ")";
}

// Note that `cte_t` is not a table, because `join` and `from` store
// `cte_ref_t`.
template <fixed_string Name, typename Statement, typename... ColumnSpecs>
struct is_cte<cte_t<Name, Statement, ColumnSpecs...>>
    : public std::true_type {};

template <fixed_string Name, typename Statement, typename... ColumnSpecs>
struct is_recursive_cte<cte_t<Name, Statement, ColumnSpecs...>>
    {
  constexpr static bool value = required_ctes_of<
      Statement>::func().contains(^^cte_ref_t<Name>);
};

template <fixed_string Name, typename Statement, typename... ColumnSpecs>
struct is_table<cte_t<Name, Statement, ColumnSpecs...>>
    : public std::true_type {};

template <fixed_string Name, typename Statement, typename... ColumnSpecs>
struct name_of<cte_t<Name, Statement, ColumnSpecs...>> {
  static constexpr fixed_string value = Name;
};

template <fixed_string Name, typename Statement, typename... ColumnSpecs>
struct nodes_of<cte_t<Name, Statement, ColumnSpecs...>> {
  using type = detail::type_vector<Statement>;
};

template <fixed_string Name, typename Statement, typename... ColumnSpecs>
struct provided_ctes_of<cte_t<Name, Statement, ColumnSpecs...>> {
  static consteval auto func() -> detail::type_info_set {
    return detail::make_type_info_set<cte_ref_t<Name>>();
  }
};

// The cte_ref_t represents the cte as table in FROM.
// The cte_t needs to be provided by WITH.
template <fixed_string Name>
struct cte_ref_t {
  template <typename Statement>
    requires(is_statement<Statement>::value and
             has_result_row<Statement>::value and
             required_tables_of<Statement>::func().empty() and
             not required_ctes_of<Statement>::func().contains(
                 ^^cte_ref_t<Name>))
  auto as(Statement statement) const -> cte_t<Name, Statement> {
    consteval {
      Statement::check_basic_consistency();
    }
    return {std::move(statement)};
  }
};

template <typename Context, fixed_string Name>
auto to_sql_string(Context& context, const cte_ref_t<Name>&)
    -> std::string {
  return name_to_sql_string(context, Name);
}

template <fixed_string Name>
struct name_of<cte_ref_t<Name>> {
  static constexpr fixed_string value = Name;
};

template <fixed_string Name>
struct provided_tables_of<cte_ref_t<Name>> {
  static consteval auto func() -> detail::type_info_set {
    return detail::make_type_info_set<cte_ref_t<Name>>();
  }
};

template <fixed_string Name>
struct required_ctes_of<cte_ref_t<Name>> {
  static consteval auto func() -> detail::type_info_set {
    return detail::make_type_info_set<cte_ref_t<Name>>();
  }
};

template <fixed_string Name>
struct required_static_ctes_of<cte_ref_t<Name>>
    : public required_ctes_of<cte_ref_t<Name>> {};

template <fixed_string Name>
auto cte() -> cte_ref_t<Name> {
  return {};
}
}  // namespace sqlpp
