module;

#include <sqlpp23/postgresql/postgresql.h>

export module sqlpp23.postgresql;

export namespace sqlpp::postgresql {
using ::sqlpp::postgresql::connection;
using ::sqlpp::postgresql::connection_config;
using ::sqlpp::postgresql::connection_pool;
using ::sqlpp::postgresql::context_t;

using ::sqlpp::postgresql::delete_from;
using ::sqlpp::postgresql::insert_into;
using ::sqlpp::postgresql::update;
}


