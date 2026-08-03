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

#ifdef BUILD_WITH_MODULES
import sqlpp23.core;
import sqlpp23.mock_db;
#else
#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/mock_db/mock_db.h>
#endif

namespace sqlpp::mock_db {
// Get configuration for test connection
inline std::shared_ptr<sqlpp::mock_db::connection_config> make_test_config(
    const std::vector<sqlpp::log_category>& categories = {log_category::all}) {
  auto config = std::make_shared<sqlpp::mock_db::connection_config>();

  config->id = "mock";
  config->debug = debug_logger(
      categories, [](sqlpp::log_category, const std::string& message) {
        std::clog << message << '\n';
      });
  return config;
}

// Starts a connection
inline ::sqlpp::mock_db::connection make_test_connection(
    const std::vector<sqlpp::log_category>& categories = {log_category::all}) {
  namespace sql = sqlpp::mock_db;

  auto config = make_test_config(categories);

  sql::connection db;
  db.connect_using(config);

  return db;
}
}  // namespace sqlpp::mock_db
