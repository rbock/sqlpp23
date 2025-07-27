module;

#include <sqlpp23/mock_db/mock_db.h>

export module sqlpp23.mock_db;

export namespace sqlpp::mock_db {
  using sqlpp::mock_db::command_result;
  using sqlpp::mock_db::connection;
  using sqlpp::mock_db::connection_config;
  using sqlpp::mock_db::context_t;
}


