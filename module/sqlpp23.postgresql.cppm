module;

#include <sqlpp23/postgresql/postgresql.h>

export module sqlpp23.postgresql;

export namespace sqlpp::postgresql {
  using sqlpp::postgresql::connection_config;
  using sqlpp::postgresql::connection;
  using sqlpp::postgresql::context_t;
}


