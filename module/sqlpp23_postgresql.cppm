module;

#include <sqlpp23/postgresql/postgresql.h>

export module sqlpp23_postgresql;

export import sqlpp23;

export namespace sqlpp::postgresql {
  using sqlpp::postgresql::connection;
}


