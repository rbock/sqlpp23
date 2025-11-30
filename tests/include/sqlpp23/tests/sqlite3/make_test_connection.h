#pragma once

/*
 * Copyright (c) 2025, Roland Bock
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

#include <iostream>
#include <memory>
#include <vector>

#ifdef SQLPP_USE_SQLCIPHER
#include <sqlcipher/sqlite3.h>
#else
#include <sqlite3.h>
#endif

#ifdef BUILD_WITH_MODULES
import sqlpp23.core;
import sqlpp23.sqlite3;
#else
#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/sqlite3/sqlite3.h>
#endif

namespace sqlpp::sqlite3 {
inline debug_logger get_debug_logger(
    const std::vector<sqlpp::log_category>& categories = {log_category::all}) {
  return debug_logger(categories,
                      [](sqlpp::log_category, const std::string& message) {
                        std::clog << message << '\n';
                      });
}

// Get configuration for test connection
inline std::shared_ptr<sqlpp::sqlite3::connection_config> make_test_config(
    const std::vector<sqlpp::log_category>& categories = {log_category::all}) {
  auto config = std::make_shared<sqlpp::sqlite3::connection_config>();

  config->path_to_database = ":memory:";
  config->flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
  config->debug = get_debug_logger(categories);
  config->use_extended_result_codes = true;

  return config;
}

inline ::sqlpp::sqlite3::connection make_test_connection(
    const std::vector<sqlpp::log_category>& categories = {log_category::all}) {
  namespace sql = sqlpp::sqlite3;

  auto config = make_test_config(categories);

  sql::connection db;
  db.connect_using(config);
  return db;
}
}  // namespace sqlpp::sqlite3
