module;

#include <sqlpp23/postgresql/postgresql.h>

export module sqlpp23.postgresql;

export namespace sqlpp::postgresql {
using ::sqlpp::postgresql::connection;
using ::sqlpp::postgresql::connection_config;
using ::sqlpp::postgresql::connection_pool;
using ::sqlpp::postgresql::pooled_connection;
using ::sqlpp::postgresql::context_t;

using ::sqlpp::postgresql::command_result;

using ::sqlpp::postgresql::delete_from;
using ::sqlpp::postgresql::insert_into;
using ::sqlpp::postgresql::update;

using ::sqlpp::postgresql::connection_exception;
using ::sqlpp::postgresql::result_exception;

using ::sqlpp::postgresql::assert_no_cast_bool_to_numeric;
using ::sqlpp::postgresql::assert_no_unsigned;

using ::sqlpp::postgresql::to_sql_string;
}


