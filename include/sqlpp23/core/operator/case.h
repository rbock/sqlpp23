#pragma once

/*
 * Copyright (c) 2015, Roland Bock
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

#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include "sqlpp23/core/type_traits/optional.h"

#include <sqlpp23/core/concepts.h>
#include <sqlpp23/core/detail/type_set.h>
#include <sqlpp23/core/detail/type_vector.h>
#include <sqlpp23/core/name/char_sequence.h>
#include <sqlpp23/core/operator/enable_as.h>
#include <sqlpp23/core/operator/enable_comparison.h>
#include <sqlpp23/core/type_traits.h>
#include <sqlpp23/core/tuple_to_sql_string.h>
#include <sqlpp23/core/logic.h>

namespace sqlpp {
namespace detail
{
  template<typename CaseDataType, typename ThenOrElse>
  struct case_data_type {
    using base_data_type = std::conditional_t<
        std::is_same_v<sqlpp::remove_optional_t<CaseDataType>, no_value_t>,
        data_type_of_t<ThenOrElse>,
        CaseDataType>;

    using type =
        std::conditional_t<is_optional<CaseDataType>::value or
                               is_optional<data_type_of_t<ThenOrElse>>::value,
                           force_optional_t<base_data_type>,
                           base_data_type>;
  };
  template<typename CaseDataType>
  struct case_data_type<CaseDataType, std::nullopt_t> {
    using base_data_type = std::conditional_t<
        std::is_same_v<sqlpp::remove_optional_t<CaseDataType>, no_value_t>,
        std::optional<no_value_t>,
        CaseDataType>;

    using type = force_optional_t<base_data_type>;
  };
  template<typename CaseDataType, typename ThenOrElse>
  using case_data_type_t = typename case_data_type<CaseDataType, ThenOrElse>::type;
}

template <typename When, typename Then>
struct when_then_pair_t {
  When _when;
  Then _then;

  using nodes = ::sqlpp::detail::type_vector<When, Then>;

  when_then_pair_t(When w, Then t) : _when(w), _then(t) {}

  when_then_pair_t(const when_then_pair_t&) = default;
  when_then_pair_t(when_then_pair_t&&) = default;
  when_then_pair_t& operator=(const when_then_pair_t&) = default;
  when_then_pair_t& operator=(when_then_pair_t&&) = default;
  ~when_then_pair_t() = default;
};

template <typename When, typename Then>
struct data_type_of<when_then_pair_t<When, Then>> : data_type_of<Then> {};

template <typename When, typename Then>
struct nodes_of<when_then_pair_t<When, Then>> {
  using type = detail::type_vector<When, Then>;
};

template <typename Context, typename When, typename Then>
auto to_sql_string(Context& context, const when_then_pair_t<When, Then>& pair) -> std::string {
  // Temporary result to enforce order of evaluation.
  auto result = " WHEN " + operand_to_sql_string(context, pair._when);
  return result + " THEN " + operand_to_sql_string(context, pair._then);
}

template <typename DataType, typename Else, typename... WhenThenPairs>
struct case_t : public enable_as, public enable_comparison {
  case_t(std::tuple<WhenThenPairs...> when_then_list, Else else_)
      : _when_then_list(std::move(when_then_list)), _else(std::move(else_)) {}

  case_t(const case_t&) = default;
  case_t(case_t&&) = default;
  case_t& operator=(const case_t&) = default;
  case_t& operator=(case_t&&) = default;
  ~case_t() = default;

  std::tuple<WhenThenPairs...> _when_then_list;
  Else _else;
};

template <typename DataType, typename Else, typename... WhenThenPairs>
struct nodes_of<case_t<DataType, Else, WhenThenPairs...>> {
  using type = detail::type_vector<WhenThenPairs..., Else>;
};

template <typename DataType, typename Else, typename... WhenThenPairs>
struct data_type_of<case_t<DataType, Else, WhenThenPairs...>>
{
  using type = DataType;
};

template <typename DataType, typename Else, typename... WhenThenPairs>
struct requires_parentheses<case_t<DataType, Else, WhenThenPairs...>> : public std::true_type {
};

template <typename Context, typename DataType, typename Else, typename... WhenThenPairs>
auto to_sql_string(Context& context, const case_t<DataType, Else, WhenThenPairs...>& t) -> std::string {
  std::string ret_val = "CASE";
  ret_val += ::sqlpp::tuple_to_sql_string(context, t._when_then_list, ::sqlpp::tuple_clause{""});
  ret_val += " ELSE " + operand_to_sql_string(context, t._else);
  ret_val += " END";
  return ret_val;
}

template <typename DataType, typename When, typename... WhenThenPairs>
class case_pending_then_t;

template <typename DataType, typename... WhenThenPairs>
class case_builder_t {
 private:
   std::tuple<WhenThenPairs...> _current_pairs;

   template<typename Else>
     using case_data_type = std::conditional_t<can_be_null<Else>::value, force_optional_t<DataType>, DataType>;

 public:
  case_builder_t(std::tuple<WhenThenPairs...> current_pairs)
      : _current_pairs(current_pairs) {}

  case_builder_t(const case_builder_t&) = default;
  case_builder_t(case_builder_t&&) = default;
  case_builder_t& operator=(const case_builder_t&) = default;
  case_builder_t& operator=(case_builder_t&&) = default;
  ~case_builder_t() = default;

  template <DynamicBoolean NewWhenCondition>
  auto when(NewWhenCondition condition)
      -> case_pending_then_t<DataType, NewWhenCondition, WhenThenPairs...> {
    return {_current_pairs, condition};
  }

  template <typename Else>
    requires(has_data_type<Else>::value and
             (std::is_same_v<remove_optional_t<DataType>, no_value_t> or
              values_are_optionally_same<DataType, Else>::value))
  auto else_(Else else_expr) -> case_t<detail::case_data_type_t<DataType, Else>,
                                       Else,
                                       WhenThenPairs...> {
    return {_current_pairs, else_expr};
  }

  template<typename = void>
    requires(not std::is_same_v<remove_optional_t<DataType>, no_value_t>)
  auto else_(std::nullopt_t null_opt_value)
      -> case_t<force_optional_t<DataType>, std::nullopt_t, WhenThenPairs...> {
    return {_current_pairs, null_opt_value};
  }
};

template <typename DataType, typename When, typename... WhenThenPairs>
class case_pending_then_t {
 private:
  std::tuple<WhenThenPairs...> _previous_pairs;
  When _condition;

 public:
  case_pending_then_t(std::tuple<WhenThenPairs...> previous_pairs,
                      When condition)
      : _previous_pairs(previous_pairs), _condition(condition) {}

  case_pending_then_t(const case_pending_then_t&) = default;
  case_pending_then_t(case_pending_then_t&&) = default;
  case_pending_then_t& operator=(const case_pending_then_t&) = default;
  case_pending_then_t& operator=(case_pending_then_t&&) = default;
  ~case_pending_then_t() = default;

  template <typename Then>
    requires(has_data_type<Then>::value and
             (std::is_same_v<remove_optional_t<DataType>, no_value_t> or
              values_are_optionally_same<DataType, Then>::value))
  auto then(Then result) {
    auto new_pair = when_then_pair_t<When, Then>{_condition, std::move(result)};
    auto new_pairs_tuple = std::tuple_cat(_previous_pairs, std::make_tuple(std::move(new_pair)));

    return case_builder_t<detail::case_data_type_t<DataType, Then>,
                          WhenThenPairs..., when_then_pair_t<When, Then>>{
        std::move(new_pairs_tuple)};
  }

  auto then(std::nullopt_t) {
    auto new_pair = when_then_pair_t<When, std::nullopt_t>{_condition, std::nullopt};
    auto new_pairs_tuple = std::tuple_cat(_previous_pairs, std::make_tuple(std::move(new_pair)));

    return case_builder_t<detail::case_data_type_t<DataType, std::nullopt_t>,
                          WhenThenPairs...,
                          when_then_pair_t<When, std::nullopt_t>>{
        std::move(new_pairs_tuple)};
  }
};

template <StaticBoolean When>
auto case_when(When when) -> case_pending_then_t<no_value_t, When> {
  return {std::tuple<>{}, std::move(when)};
}
}  // namespace sqlpp
