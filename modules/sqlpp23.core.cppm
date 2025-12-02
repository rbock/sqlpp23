/*
 * Copyright (c) 2025, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
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

module;

#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/core/database/connection_pool.h>
#include <sqlpp23/core/detail/parse_date_time.h>
export module sqlpp23.core;

export namespace sqlpp {
// basics:
using ::sqlpp::column_t;
using ::sqlpp::table_t;
using ::sqlpp::table_as_t;
using ::sqlpp::verbatim_t;
using ::sqlpp::verbatim_table;
using ::sqlpp::table_columns;
using ::sqlpp::all_of;
using ::sqlpp::enable_join;
using ::sqlpp::join;
using ::sqlpp::inner_join;
using ::sqlpp::left_outer_join;
using ::sqlpp::right_outer_join;
using ::sqlpp::full_outer_join;
using ::sqlpp::cross_join;
using ::sqlpp::parameter;
using ::sqlpp::parameter_t; // TODO remove?
using ::sqlpp::verbatim;
using ::sqlpp::parameterized_verbatim;
using ::sqlpp::schema;
using ::sqlpp::schema_qualified_table;
using ::sqlpp::star;
using ::sqlpp::value;
using ::sqlpp::no_value_t;
using ::sqlpp::noop;
using ::sqlpp::wrapped_static_assert; // TODO: Move

// data types
using ::sqlpp::boolean;
using ::sqlpp::integral;
using ::sqlpp::unsigned_integral;
using ::sqlpp::floating_point;
using ::sqlpp::text;
using ::sqlpp::blob;
using ::sqlpp::date;
using ::sqlpp::timestamp;
using ::sqlpp::time;

// operators:arithmetic
using ::sqlpp::operator+;
using ::sqlpp::operator-;
using ::sqlpp::operator*;
using ::sqlpp::operator/;
using ::sqlpp::operator%;

// operators:as
using ::sqlpp::as;
using ::sqlpp::enable_as;

// operators:assign
using ::sqlpp::assign;

// operators:bit
using ::sqlpp::operator&;
using ::sqlpp::operator|;
using ::sqlpp::operator^;
using ::sqlpp::operator~;
using ::sqlpp::operator<<;
using ::sqlpp::operator>>;

// operators:comparison
using ::sqlpp::operator<;
using ::sqlpp::operator<=;
using ::sqlpp::operator==;
using ::sqlpp::operator!=;
using ::sqlpp::operator>=;
using ::sqlpp::operator>;
using ::sqlpp::operator==;
using ::sqlpp::operator==;
using ::sqlpp::is_null;
using ::sqlpp::is_not_null;
using ::sqlpp::is_distinct_from;
using ::sqlpp::is_not_distinct_from;
using ::sqlpp::like;
using ::sqlpp::enable_comparison;

// operators:logical
using ::sqlpp::operator&&;
using ::sqlpp::operator||;
using ::sqlpp::operator!;

// operators:misc
using ::sqlpp::any;
using ::sqlpp::between;
using ::sqlpp::case_when;
using ::sqlpp::cast;
using ::sqlpp::cast_as_t; // TODO: Remove?
using ::sqlpp::exists;
using ::sqlpp::in;
using ::sqlpp::not_in;
using ::sqlpp::asc;
using ::sqlpp::desc;
using ::sqlpp::sort_type; // enum
using ::sqlpp::order;
using ::sqlpp::when_then_pair_t; // TODO: Remove?

// clauses:
using ::sqlpp::cte;
using ::sqlpp::default_value;
using ::sqlpp::default_value_t;
using ::sqlpp::delete_from;
using ::sqlpp::for_update;
using ::sqlpp::from;
using ::sqlpp::group_by;
using ::sqlpp::having;
using ::sqlpp::insert;
using ::sqlpp::insert_columns;
using ::sqlpp::insert_default_values;
using ::sqlpp::insert_into;
using ::sqlpp::insert_set;
using ::sqlpp::into;
using ::sqlpp::limit;
using ::sqlpp::offset;
using ::sqlpp::on_conflict;
using ::sqlpp::order_by;
using ::sqlpp::returning;
using ::sqlpp::select;
using ::sqlpp::select_columns;
using ::sqlpp::simple_column;
using ::sqlpp::single_table;
using ::sqlpp::star;
using ::sqlpp::star_t;
using ::sqlpp::statement_t;
using ::sqlpp::truncate;
using ::sqlpp::union_all;
using ::sqlpp::union_all_t;
using ::sqlpp::union_distinct;
using ::sqlpp::union_order_by;
using ::sqlpp::update;
using ::sqlpp::update_set;
using ::sqlpp::using_;
using ::sqlpp::where;
using ::sqlpp::with;
using ::sqlpp::with_result_type_of;
using ::sqlpp::no_insert_value_list_t;
using ::sqlpp::having_t; // TODO: remove
using ::sqlpp::no_having_t;
using ::sqlpp::no_limit_t;
using ::sqlpp::no_offset_t;
using ::sqlpp::no_into_t;
using ::sqlpp::no_from_t;
using ::sqlpp::no_order_by_t;
using ::sqlpp::no_group_by_t;
using ::sqlpp::no_where_t;
using ::sqlpp::no_select_column_list_t;
using ::sqlpp::no_single_table_t;
using ::sqlpp::no_returning_t;
using ::sqlpp::no_on_conflict_t;
using ::sqlpp::no_with_t;
using ::sqlpp::no_union_t;
using ::sqlpp::no_union_order_by_t;
using ::sqlpp::no_using_t;
using ::sqlpp::no_update_set_list_t;
using ::sqlpp::cte_ref_t; // TODO: remove?
using ::sqlpp::select_ref_t; // TODO: remove?
using ::sqlpp::cte_union_t; // TODO: remove?
using ::sqlpp::with_t; // TODO: remove?

// select flags
using ::sqlpp::all;
using ::sqlpp::all_t;
using ::sqlpp::distinct;
using ::sqlpp::distinct_t;

// aggregate functions
using ::sqlpp::star;
using ::sqlpp::avg;
using ::sqlpp::count;
using ::sqlpp::max;
using ::sqlpp::min;
using ::sqlpp::sum;
using ::sqlpp::over;

// functions
using ::sqlpp::coalesce;
using ::sqlpp::concat;
using ::sqlpp::current_date;
using ::sqlpp::current_time;
using ::sqlpp::current_timestamp;
using ::sqlpp::flatten;
using ::sqlpp::get_sql_name;
using ::sqlpp::lower;
using ::sqlpp::make_table_ref;
using ::sqlpp::trim;
using ::sqlpp::upper;

// database
using ::sqlpp::isolation_level;
using ::sqlpp::start_transaction;
using ::sqlpp::exception;
using ::sqlpp::connection_check;
using ::sqlpp::normal_connection;
using ::sqlpp::pooled_connection;

// query
using ::sqlpp::dynamic;
using ::sqlpp::dynamic_t;

// serialization
using ::sqlpp::to_sql_string;

// logging
using ::sqlpp::log_category;
using ::sqlpp::log_function_t;
using ::sqlpp::debug_logger;

// type_traits
using ::sqlpp::no_of_result_columns;
using ::sqlpp::make_char_sequence_t; // TODO: Remove?
using ::sqlpp::can_be_null;
using ::sqlpp::data_type_of_t;
using ::sqlpp::get_result_row_t;
using ::sqlpp::has_data_type;
using ::sqlpp::has_data_type_v;
using ::sqlpp::has_name_tag;
using ::sqlpp::is_aggregate_neutral;
using ::sqlpp::has_name_tag_v;
using ::sqlpp::has_result_row;
using ::sqlpp::is_boolean;
using ::sqlpp::is_floating_point;
using ::sqlpp::is_integral;
using ::sqlpp::provided_ctes_of_t;
using ::sqlpp::is_numeric;
using ::sqlpp::is_optional;
using ::sqlpp::is_prepared_statement;
using ::sqlpp::is_prepared_statement_v;
using ::sqlpp::is_raw_select_flag;
using ::sqlpp::is_table;
using ::sqlpp::is_text;
using ::sqlpp::is_unsigned_integral;
using ::sqlpp::make_parameter_list_t;
using ::sqlpp::parameter_value_t;
using ::sqlpp::have_correct_static_cte_dependencies;
using ::sqlpp::parameters_of_t;
using ::sqlpp::remove_as_t;
using ::sqlpp::requires_parentheses;
using ::sqlpp::remove_any_t;
using ::sqlpp::remove_dynamic_t;
using ::sqlpp::remove_optional_t;
using ::sqlpp::required_tables_of;
using ::sqlpp::required_tables_of_t;
using ::sqlpp::required_insert_columns_of_t;
using ::sqlpp::provided_static_ctes_of_t;
using ::sqlpp::is_aggregate_function;
using ::sqlpp::provided_static_ctes_of_t;
using ::sqlpp::is_as_expression;
using ::sqlpp::is_data_type;
using ::sqlpp::contains_aggregate_function;
using ::sqlpp::is_aggregate_expression;
using ::sqlpp::is_non_aggregate_expression;
using ::sqlpp::assert_no_parameters_t; // TODO: constraint test?
using ::sqlpp::is_time;
using ::sqlpp::values_are_comparable;
using ::sqlpp::nodes_of;
using ::sqlpp::wrong;
using ::sqlpp::compatibility_check;
using ::sqlpp::data_type_of;
using ::sqlpp::is_timestamp;
using ::sqlpp::is_blob;
using ::sqlpp::is_boolean;
using ::sqlpp::is_date;
using ::sqlpp::nodes_of_t;
using ::sqlpp::select_column_has_name;
using ::sqlpp::has_enabled_over;
using ::sqlpp::has_enabled_comparison;
using ::sqlpp::has_enabled_as;
using ::sqlpp::has_enabled_join;
using ::sqlpp::provided_tables_of_t;
using ::sqlpp::has_default;
using ::sqlpp::name_tag_of_t;
using ::sqlpp::provided_static_tables_of_t;
using ::sqlpp::provided_optional_tables_of_t;
using ::sqlpp::required_static_tables_of_t;
using ::sqlpp::required_tables_of_t;
using ::sqlpp::required_static_ctes_of_t;
using ::sqlpp::required_ctes_of_t;
using ::sqlpp::select_column_data_type_of_t;
using ::sqlpp::is_raw_table;
using ::sqlpp::is_result_clause;
using ::sqlpp::is_column;
using ::sqlpp::is_cte;
using ::sqlpp::is_recursive_cte;
using ::sqlpp::known_aggregate_columns_of_t;
using ::sqlpp::known_static_aggregate_columns_of_t;
using ::sqlpp::have_correct_cte_dependencies;

// constraints
using ::sqlpp::assert_insert_values_t; // Need constraint test
using ::sqlpp::assert_no_unknown_tables_in_column_list_t; // Need constraint test
using ::sqlpp::assert_all_required_assignments_t;
using ::sqlpp::assert_all_required_columns_t;
using ::sqlpp::assert_no_unknown_tables_in_insert_assignments_t;
using ::sqlpp::assert_columns_selected_t;
using ::sqlpp::assert_correct_order_by_aggregates_t;
using ::sqlpp::assert_correct_order_by_aggregates_with_group_by_t;
using ::sqlpp::assert_correct_static_order_by_aggregates_with_group_by_t;
using ::sqlpp::assert_having_all_aggregates_t;
using ::sqlpp::assert_having_all_static_aggregates_t;
using ::sqlpp::assert_into_t;
using ::sqlpp::assert_no_duplicate_table_providers_t;
using ::sqlpp::assert_no_unknown_columns_in_union_sort_order_t;
using ::sqlpp::assert_no_unknown_ctes_t;
using ::sqlpp::assert_no_unknown_static_tables_in_group_by_t;
using ::sqlpp::assert_no_unknown_static_tables_in_having_t;
using ::sqlpp::assert_no_unknown_static_tables_in_on_conflict_do_update_t;
using ::sqlpp::assert_no_unknown_static_tables_in_order_by_t;
using ::sqlpp::assert_no_unknown_static_tables_in_selected_columns_t;
using ::sqlpp::assert_no_unknown_static_tables_in_where_t;
using ::sqlpp::assert_no_unknown_tables_in_group_by_t;
using ::sqlpp::assert_no_unknown_tables_in_having_t;
using ::sqlpp::assert_no_unknown_tables_in_on_conflict_do_update_t;
using ::sqlpp::assert_no_unknown_tables_in_order_by_t;
using ::sqlpp::assert_no_unknown_tables_in_returning_columns_t;
using ::sqlpp::assert_no_unknown_tables_in_selected_columns_t;
using ::sqlpp::assert_no_unknown_tables_in_update_assignments_t;
using ::sqlpp::assert_no_unknown_tables_in_where_t;
using ::sqlpp::assert_on_conflict_action_t;
using ::sqlpp::assert_returning_columns_contain_no_aggregates_t;
using ::sqlpp::assert_select_columns_all_aggregates_t;
using ::sqlpp::assert_select_columns_with_group_by_are_aggregates_t;
using ::sqlpp::assert_select_columns_with_group_by_match_static_aggregates_t;
using ::sqlpp::assert_single_table_provided_t;
using ::sqlpp::assert_update_assignments_t;
using ::sqlpp::check_compatibility;
using ::sqlpp::check_basic_consistency;
using ::sqlpp::check_prepare_consistency;
using ::sqlpp::check_run_consistency;
using ::sqlpp::consistency_check_t;
using ::sqlpp::consistent_t;
using ::sqlpp::prepare_check_t;
using ::sqlpp::statement_consistency_check_t;
using ::sqlpp::statement_prepare_check_t;
using ::sqlpp::statement_run_check_t;
}

export namespace sqlpp::alias {
using ::sqlpp::alias::avg_;
using ::sqlpp::alias::count_;
using ::sqlpp::alias::distinct_avg_;
using ::sqlpp::alias::distinct_count_;
using ::sqlpp::alias::distinct_max_;
using ::sqlpp::alias::distinct_min_;
using ::sqlpp::alias::distinct_sum_;
using ::sqlpp::alias::exists_;
using ::sqlpp::alias::max_;
using ::sqlpp::alias::min_;
using ::sqlpp::alias::sum_;
}

export namespace sqlpp::chrono {
using ::sqlpp::chrono::sys_microseconds;
}

export namespace sqlpp::detail {
// detail
using ::sqlpp::detail::circular_buffer;
using ::sqlpp::detail::parse_date;
using ::sqlpp::detail::parse_time;
using ::sqlpp::detail::parse_timestamp;
using ::sqlpp::detail::type_set;
using ::sqlpp::detail::type_vector;
using ::sqlpp::detail::get_last_if_t;
}

export namespace sqlpp::logic {
using ::sqlpp::logic::logic_helper;
using ::sqlpp::logic::all;
using ::sqlpp::logic::any;
using ::sqlpp::logic::none;
}
