module;

#include <sqlpp23/sqlpp23.h>

export module sqlpp23;

export namespace sqlpp {
// basics:
using ::sqlpp::column_t;
using ::sqlpp::table_t;
using ::sqlpp::table_columns;

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

// clauses:
using ::sqlpp::delete_from;
using ::sqlpp::insert;
using ::sqlpp::insert_into;
using ::sqlpp::select;
using ::sqlpp::update;
}

export namespace sqlpp::detail {
// detail
using sqlpp::detail::type_set;
}

