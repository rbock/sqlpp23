module;

#include <sqlpp23/sqlite3/sqlite3.h>

export module sqlpp23.sqlite3;

export namespace sqlpp::sqlite3 {
using ::sqlpp::sqlite3::connection;
using ::sqlpp::sqlite3::connection_config;
using ::sqlpp::sqlite3::connection_pool;
using ::sqlpp::sqlite3::pooled_connection;
using ::sqlpp::sqlite3::context_t;

using ::sqlpp::sqlite3::command_result;
using ::sqlpp::sqlite3::exception;

using ::sqlpp::sqlite3::delete_from;
using ::sqlpp::sqlite3::update;
using ::sqlpp::sqlite3::insert_into;
using ::sqlpp::sqlite3::insert_or_replace;
using ::sqlpp::sqlite3::insert_or_ignore;

using ::sqlpp::sqlite3::assert_no_cast_to_date_time;
using ::sqlpp::sqlite3::assert_no_any_t;

using ::sqlpp::sqlite3::to_sql_string;
}
