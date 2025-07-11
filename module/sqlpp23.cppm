module;

#include <sqlpp23/sqlpp23.h>

export module sqlpp23;

export namespace sqlpp {
// basics:
using ::sqlpp::column_t;
using ::sqlpp::table_t;
using ::sqlpp::table_columns;
using ::sqlpp::all_of;
using ::sqlpp::join;
using ::sqlpp::inner_join;
using ::sqlpp::left_outer_join;
using ::sqlpp::right_outer_join;
using ::sqlpp::full_outer_join;
using ::sqlpp::cross_join;
using ::sqlpp::parameter;
using ::sqlpp::verbatim;
using ::sqlpp::parameterized_verbatim;
using ::sqlpp::schema;
using ::sqlpp::schema_qualified_table;
using ::sqlpp::star;
using ::sqlpp::value;

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

// operators:logical
using ::sqlpp::operator&&;
using ::sqlpp::operator||;
using ::sqlpp::operator!;

// operators:misc
using ::sqlpp::any;
using ::sqlpp::between;
using ::sqlpp::case_when;
using ::sqlpp::cast;
using ::sqlpp::exists;
using ::sqlpp::in;
using ::sqlpp::not_in;
using ::sqlpp::asc;
using ::sqlpp::desc;
using ::sqlpp::sort_type; // enum
using ::sqlpp::order;

// clauses:
using ::sqlpp::delete_from;
using ::sqlpp::insert;
using ::sqlpp::insert_into;
using ::sqlpp::select;
using ::sqlpp::truncate;
using ::sqlpp::update;

using ::sqlpp::cte;
using ::sqlpp::for_update;
using ::sqlpp::from;
using ::sqlpp::group_by;
using ::sqlpp::having;
using ::sqlpp::insert_columns;
using ::sqlpp::insert_default_values;
using ::sqlpp::insert_set;
using ::sqlpp::into;
using ::sqlpp::limit;
using ::sqlpp::offset;
using ::sqlpp::on_conflict;
using ::sqlpp::order_by;
using ::sqlpp::returning;
using ::sqlpp::select_columns;
using ::sqlpp::single_table;
using ::sqlpp::union_all;
using ::sqlpp::union_distinct;
using ::sqlpp::update_set;
using ::sqlpp::using_;
using ::sqlpp::where;
using ::sqlpp::with;

// functions
using sqlpp::coalesce;
using sqlpp::concat;
using sqlpp::current_date;
using sqlpp::current_time;
using sqlpp::current_timestamp;
using sqlpp::lower;
using sqlpp::trim;
using sqlpp::upper;

}

export namespace sqlpp::detail {
// detail
using sqlpp::detail::type_set;
}

