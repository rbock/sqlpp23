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

#include <sqlpp23/core/concepts.h>
#include <sqlpp23/core/detail/type_set.h>
#include <sqlpp23/core/detail/type_vector.h> // For type_vector_cat_t
#include <sqlpp23/core/name/char_sequence.h>
#include <sqlpp23/core/operator/enable_as.h>
#include <sqlpp23/core/operator/enable_comparison.h>
#include <sqlpp23/core/type_traits.h>
#include <sqlpp23/core/tuple_to_sql_string.h> // For tuple_to_sql_string and tuple_clause
#include <sqlpp23/core/logic.h> // For ::sqlpp::logic::any

#include <optional> // Required for std::nullopt_t
#include <string>  // Required for std::string
#include <tuple>   // Required for std::tuple
#include <utility> // Required for std::pair

namespace sqlpp {
namespace detail {

template <typename WhenThenPairsTuple, typename ElseType>
struct nodes_of_case_impl; // Primary template

template <typename... Pairs, typename ElseType>
struct nodes_of_case_impl<std::tuple<Pairs...>, ElseType> {
  // Requires each type in 'Pairs...' to have a '::nodes' type alias
  // that is a type_vector.
  // Also assumes ElseType is not void. If ElseType could be void or a marker for no ELSE,
  // then type_vector<ElseType> might need to be conditional, e.g.
  // type_vector_cat_t<typename Pairs::nodes..., MaybeElseVector>
  // For now, assume ElseType is always a valid type for type_vector.
  using type = ::sqlpp::detail::type_vector_cat_t<typename Pairs::nodes..., ::sqlpp::detail::type_vector<ElseType>>;
};

// New helper to check if any type in a type_vector can_be_null using ::sqlpp::logic::any
template <typename TypeVector>
struct can_any_in_vector_be_null; // Primary template

template <typename... Types>
struct can_any_in_vector_be_null<::sqlpp::detail::type_vector<Types...>>
    : public ::sqlpp::logic::any<can_be_null<Types>::value...> {};

  // Helper to collect all Then types from WhenThenPairsTuple and the ElseType
  template <typename WhenThenPairsTuple, typename ElseType_>
  struct get_then_and_else_expressions_for_nullability_check;

  template <typename... PairTypes, typename ElseType_>
  struct get_then_and_else_expressions_for_nullability_check<std::tuple<PairTypes...>, ElseType_> {
    template <typename Tuple, typename Else, typename... Acc>
    struct extract_thens_for_null_check_impl;

    template <typename Else_, typename... Acc>
    struct extract_thens_for_null_check_impl<std::tuple<>, Else_, Acc...> {
      using type = ::sqlpp::detail::type_vector<Acc..., Else_>;
    };

    template <typename W, typename T, typename... RestPairs, typename Else_, typename... Acc>
    struct extract_thens_for_null_check_impl<std::tuple<::sqlpp::when_then_pair_t<W,T>, RestPairs...>, Else_, Acc...> {
      using type = typename extract_thens_for_null_check_impl<std::tuple<RestPairs...>, Else_, Acc..., T>::type;
    };
    
    using type = typename extract_thens_for_null_check_impl<std::tuple<PairTypes...>, ElseType_>::type;
  };

template <typename WhenThenPairsTuple, typename ElseType, typename ResultType>
struct derive_case_data_type_and_nullability {
  using expressions_for_null_check = 
      typename ::sqlpp::detail::get_then_and_else_expressions_for_nullability_check<WhenThenPairsTuple, ElseType>::type;
  
  static constexpr bool result_can_be_null = 
      ::sqlpp::detail::can_any_in_vector_be_null<expressions_for_null_check>::value;

  using base_cpp_type = ::sqlpp::data_type_of_t<ResultType>;
  using type = std::conditional_t<result_can_be_null,
                                  ::sqlpp::force_optional_t<base_cpp_type>,
                                  base_cpp_type>;
};

} // namespace detail

struct undefined_result_type_marker {};

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

template <typename Context, typename When, typename Then>
auto to_sql_string(Context& context, const when_then_pair_t<When, Then>& pair) -> std::string {
  return " WHEN " + operand_to_sql_string(context, pair._when) +
         " THEN " + operand_to_sql_string(context, pair._then);
}

template <typename WhenThenList, typename Else, typename ResultType>
struct case_t : public enable_as, public enable_comparison {
  case_t(WhenThenList when_then_list, Else else_) // Constructor signature remains the same for now
      : _when_then_list(when_then_list), _else(else_) {}

  case_t(const case_t&) = default;
  case_t(case_t&&) = default;
  case_t& operator=(const case_t&) = default;
  case_t& operator=(case_t&&) = default;
  ~case_t() = default;

  WhenThenList _when_then_list;
  Else _else;
  // ResultType is a tag, not stored as a member for now
};

template <typename WhenThenPairsTuple, typename Else, typename ResultType>
struct nodes_of<case_t<WhenThenPairsTuple, Else, ResultType>> {
  using type = typename ::sqlpp::detail::nodes_of_case_impl<WhenThenPairsTuple, Else>::type;
};

template <typename WhenThenPairsTuple, typename Else, typename ResultType>
struct data_type_of<case_t<WhenThenPairsTuple, Else, ResultType>>
    : public ::sqlpp::detail::derive_case_data_type_and_nullability<WhenThenPairsTuple, Else, ResultType> {};

template <typename WhenThenPairsTuple, typename Else, typename ResultType>
struct requires_parentheses<case_t<WhenThenPairsTuple, Else, ResultType>> : public std::true_type {
};

template <typename Context, typename WhenThenList, typename Else, typename ResultType>
auto to_sql_string(Context& context, const case_t<WhenThenList, Else, ResultType>& t) -> std::string {
  std::string ret_val = "CASE";
  ret_val += ::sqlpp::tuple_to_sql_string(context, t._when_then_list, ::sqlpp::tuple_clause{""});
  ret_val += " ELSE " + operand_to_sql_string(context, t._else);
  ret_val += " END";
  return ret_val;
}

template <typename CurrentPairsTuple, typename ResultType>
class case_builder_t {
 private:
  CurrentPairsTuple _current_pairs;
  // ResultType _result_type_tag; // Not storing as a member for now

 public:
  case_builder_t(CurrentPairsTuple current_pairs) // ResultType is not used in constructor for now
      : _current_pairs(current_pairs) {}

  case_builder_t(const case_builder_t&) = default;
  case_builder_t(case_builder_t&&) = default;
  case_builder_t& operator=(const case_builder_t&) = default;
  case_builder_t& operator=(case_builder_t&&) = default;
  ~case_builder_t() = default;

  template <typename NewWhenCondition>
    requires(StaticBoolean<NewWhenCondition>::value)
  auto when(NewWhenCondition condition)
      -> case_pending_then_t<NewWhenCondition, CurrentPairsTuple, ResultType> {
    return {condition, _current_pairs}; // Assuming case_pending_then_t constructor will adapt
  }

  template <typename ElseExpression>
    requires(::sqlpp::values_are_comparable<ResultType, ElseExpression>::value)
  auto else_(ElseExpression else_expr)
      -> case_t<CurrentPairsTuple, ElseExpression, ResultType> { // Assuming case_t will adapt
    return {_current_pairs, else_expr};
  }

  template <typename ElseType = std::nullopt_t> // Added ElseType for clarity in requires
    requires(::sqlpp::values_are_comparable<ResultType, ElseType>::value)
  auto else_(std::nullopt_t null_opt_value)
      -> case_t<CurrentPairsTuple, std::nullopt_t, ResultType> { // Assuming case_t will adapt
    return {_current_pairs, null_opt_value};
  }
};

template <typename WhenCondition, typename PreviousPairsTuple, typename ResultTypeSoFar = ::sqlpp::undefined_result_type_marker>
class case_pending_then_t {
 private:
  WhenCondition _condition;
  PreviousPairsTuple _previous_pairs;

 public:
  case_pending_then_t(WhenCondition condition,
                      PreviousPairsTuple previous_pairs) // ResultTypeSoFar is a tag, not passed to constructor
      : _condition(condition), _previous_pairs(previous_pairs) {}

  case_pending_then_t(const case_pending_then_t&) = default;
  case_pending_then_t(case_pending_then_t&&) = default;
  case_pending_then_t& operator=(const case_pending_then_t&) = default;
  case_pending_then_t& operator=(case_pending_then_t&&) = default;
  ~case_pending_then_t() = default;

  template <typename ThenResult>
    requires(::sqlpp::has_data_type<ThenResult>::value)
  auto then(ThenResult result) { // Return type will be deduced or specified with decltype
    auto new_pair = when_then_pair_t<WhenCondition, ThenResult>{_condition, result};
    auto new_pairs_tuple = std::tuple_cat(_previous_pairs, std::make_tuple(new_pair));

    if constexpr (std::is_same_v<ResultTypeSoFar, ::sqlpp::undefined_result_type_marker>) {
      // This is the first .then() call, establishing ResultType
      return case_builder_t<decltype(new_pairs_tuple), ThenResult>{new_pairs_tuple};
    } else {
      // Subsequent .then() call, ResultTypeSoFar is established
      static_assert(::sqlpp::values_are_comparable<ResultTypeSoFar, ThenResult>::value,
                    "Type in .then() is not comparable with the type of the first .then() clause in CASE expression");
      return case_builder_t<decltype(new_pairs_tuple), ResultTypeSoFar>{new_pairs_tuple};
    }
  }
};

template <StaticBoolean When>
auto case_when(When when) -> case_pending_then_t<When, std::tuple<>, ::sqlpp::undefined_result_type_marker> {
  return {std::move(when), std::tuple<>{}};
}
}  // namespace sqlpp
