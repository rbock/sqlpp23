#pragma once

/*
 * Copyright (c) 2024, Roland Bock
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

#include <sqlpp26/core/concepts.h>
#include <sqlpp26/core/detail/type_vector.h>
#include <sqlpp26/core/query/dynamic.h>
#include <sqlpp26/core/type_traits.h>
#include <sqlpp26/core/unconditional.h>
#include <sqlpp26/core/detail/type_set.h>

namespace sqlpp {
// Join types.
struct cross_join_t {
  static constexpr const char* _name = " CROSS JOIN ";
};

struct inner_join_t {
  static constexpr const char* _name = " INNER JOIN ";
};

struct left_outer_join_t {
  static constexpr const char* _name = " LEFT OUTER JOIN ";
};

struct right_outer_join_t {
  static constexpr const char* _name = " RIGHT OUTER JOIN ";
};

struct full_outer_join_t {
  static constexpr const char* _name = " FULL OUTER JOIN ";
};

template <typename Lhs, typename JoinType, typename Rhs, typename Condition>
struct join_t;

template <typename Lhs, typename JoinType, typename Rhs>
class pre_join_t;

// This turns a type info set into an array which can be used in template-for
template<typename T>
consteval auto provided_tables_of_as_array() {
    auto set = sqlpp::provided_tables_of<T>::func();
    std::array<std::meta::info, sqlpp::provided_tables_of<T>::func().size()> arr{};
    std::copy(set.begin(), set.end(), arr.begin());
    return arr;
}

template<typename Lhs, typename Rhs>
consteval bool are_table_names_disjoint() {
    std::flat_set<std::string_view> all_names;

    // Needs to be static to ensure constant memory location across invocations.
    static constexpr auto lhs = provided_tables_of_as_array<Lhs>();
    static constexpr auto rhs = provided_tables_of_as_array<Rhs>();

    template for (constexpr auto info : lhs) {
        all_names.insert(name_of_v<typename [: info :]>);
    }

    template for (constexpr auto info : rhs) {
        auto [_, inserted] = all_names.insert(name_of_v<typename [: info :]>);
        if (!inserted) return false;
    }

    return true;
}

template <StaticTable Lhs, DynamicTable Rhs>
inline constexpr bool can_be_joined_v =
    required_tables_of<Lhs>::func().empty() and required_tables_of<Rhs>::func().empty() and
    are_table_names_disjoint<Lhs, Rhs>();

template <StaticTable Lhs, DynamicTable Rhs>
  requires(can_be_joined_v<Lhs, Rhs>)
auto join(Lhs lhs, Rhs rhs)
    -> pre_join_t<table_ref_t<Lhs>, inner_join_t, table_ref_t<Rhs>>;

template <StaticTable Lhs, DynamicTable Rhs>
  requires(can_be_joined_v<Lhs, Rhs>)
auto inner_join(Lhs lhs, Rhs rhs)
    -> pre_join_t<table_ref_t<Lhs>, inner_join_t, table_ref_t<Rhs>>;

template <StaticTable Lhs, DynamicTable Rhs>
  requires(can_be_joined_v<Lhs, Rhs>)
auto left_outer_join(Lhs lhs, Rhs rhs)
    -> pre_join_t<table_ref_t<Lhs>, left_outer_join_t, table_ref_t<Rhs>>;

template <StaticTable Lhs, DynamicTable Rhs>
  requires(can_be_joined_v<Lhs, Rhs>)
auto right_outer_join(Lhs lhs, Rhs rhs)
    -> pre_join_t<table_ref_t<Lhs>, right_outer_join_t, table_ref_t<Rhs>>;

template <StaticTable Lhs, DynamicTable Rhs>
  requires(can_be_joined_v<Lhs, Rhs>)
auto full_outer_join(Lhs lhs, Rhs rhs)
    -> pre_join_t<table_ref_t<Lhs>, full_outer_join_t, table_ref_t<Rhs>>;

template <StaticTable Lhs, DynamicTable Rhs>
  requires(can_be_joined_v<Lhs, Rhs>)
auto cross_join(Lhs lhs, Rhs rhs) -> join_t<table_ref_t<Lhs>,
                                            cross_join_t,
                                            table_ref_t<Rhs>,
                                            unconditional_t>;

}  // namespace sqlpp
