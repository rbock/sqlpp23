module;

#include <sqlpp23/mysql/mysql.h>

export module sqlpp23.mysql;

export namespace sqlpp::mysql {
using ::sqlpp::mysql::connection;
using ::sqlpp::mysql::connection_config;
using ::sqlpp::mysql::connection_pool;
using ::sqlpp::mysql::pooled_connection;
using ::sqlpp::mysql::context_t;

using ::sqlpp::mysql::command_result;
using ::sqlpp::mysql::exception;

using ::sqlpp::mysql::scoped_library_initializer_t;
using ::sqlpp::mysql::global_library_init;

using ::sqlpp::mysql::delete_from;
using ::sqlpp::mysql::update;

using ::sqlpp::mysql::assert_no_bool_cast;
using ::sqlpp::mysql::assert_no_full_outer_join_t;

using ::sqlpp::mysql::to_sql_string;
}


