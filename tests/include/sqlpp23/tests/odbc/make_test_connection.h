#pragma once

/*
 * Copyright (c) 2026, Leander Schulten
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

#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

#ifdef BUILD_WITH_MODULES
import sqlpp23.core;
import sqlpp23.odbc;
#else
#include <sqlpp23/odbc/odbc.h>
#include <sqlpp23/sqlpp23.h>
#endif

namespace sqlpp::odbc {
inline debug_logger get_debug_logger(
    const std::vector<sqlpp::log_category>& categories = {log_category::all}) {
  return debug_logger(categories,
                      [](sqlpp::log_category, const std::string& message) {
                        std::clog << message << '\n';
                      });
}

// Get configuration for test connection. The connection string is taken from
// the environment (SQLPP_ODBC_CONNECTION_STRING) since ODBC tests require an
// externally configured driver and database.
inline std::shared_ptr<sqlpp::odbc::connection_config> make_test_config(
    const std::vector<sqlpp::log_category>& categories = {log_category::all}) {
  auto config = std::make_shared<sqlpp::odbc::connection_config>();

  if (const char* value = std::getenv("SQLPP_ODBC_CONNECTION_STRING")) {
    config->source = sqlpp::odbc::connection_string{value};
  }
  config->debug = get_debug_logger(categories);

  return config;
}

inline ::sqlpp::odbc::connection make_test_connection(
    const std::vector<sqlpp::log_category>& categories = {log_category::all}) {
  namespace sql = sqlpp::odbc;

  auto config = make_test_config(categories);

  sql::connection db;
  db.connect_using(config);
  return db;
}
}  // namespace sqlpp::odbc
