/*
 * Copyright (c) 2026
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
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

#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <duckdb.h>

#ifdef BUILD_WITH_MODULES
import sqlpp23.core;
import sqlpp23.duck_db;
#else
#include <sqlpp23/duck_db/duck_db.h>
#include <sqlpp23/sqlpp23.h>
#endif

namespace sqlpp::duck_db {

inline debug_logger get_debug_logger(
    const std::vector<sqlpp::log_category>& categories = {log_category::all}) {
  return debug_logger(categories,
                      [](sqlpp::log_category, const std::string& message) {
                        std::clog << message << '\n';
                      });
}

// Configuration for a private in-memory test database.
inline std::shared_ptr<sqlpp::duck_db::connection_config> make_test_config(
    const std::vector<sqlpp::log_category>& categories = {log_category::all}) {
  auto config = std::make_shared<sqlpp::duck_db::connection_config>();

  config->path_to_database = ":memory:";
  config->debug = get_debug_logger(categories);

  return config;
}

// DuckDB in-memory databases are private to a single connection. Tests that need
// more than one connection to see the same data have to use a file.
inline std::shared_ptr<sqlpp::duck_db::connection_config>
make_test_file_config(const std::string& path,
                      const std::vector<sqlpp::log_category>& categories = {}) {
  auto config = std::make_shared<sqlpp::duck_db::connection_config>();

  config->path_to_database = path;
  config->debug = get_debug_logger(categories);

  return config;
}

inline ::sqlpp::duck_db::connection make_test_connection(
    const std::vector<sqlpp::log_category>& categories = {log_category::all}) {
  namespace sql = sqlpp::duck_db;

  auto config = make_test_config(categories);

  sql::connection db;
  db.connect_using(config);
  return db;
}

// Connection to a file based database. The caller is responsible for removing
// the file, e.g. with std::remove(path.c_str()).
inline ::sqlpp::duck_db::connection make_test_file_connection(
    const std::string& path,
    const std::vector<sqlpp::log_category>& categories = {}) {
  namespace sql = sqlpp::duck_db;

  auto config = make_test_file_config(path, categories);

  sql::connection db;
  db.connect_using(config);
  return db;
}

}  // namespace sqlpp::duck_db
