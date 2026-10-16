#pragma once

/*
 * Copyright (c) 2026
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
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

#include <string>
#include <utility>
#include <vector>

#include <sqlpp23/core/debug_logger.h>

namespace sqlpp::duck_db {

// DuckDB opens a database (one or more files, or an anonymous in-memory
// database) and hands out connections from it. path_to_database defaults to
// ":memory:", which gives an anonymous database that is private to the
// resulting connection. Connection pools that share data therefore need a file
// based database.
struct connection_config {
  using config_option_t = std::pair<std::string, std::string>;

  connection_config() = default;
  connection_config(const connection_config&) = default;
  connection_config(connection_config&&) = default;
  connection_config& operator=(const connection_config&) = default;
  connection_config& operator=(connection_config&&) = default;

  explicit connection_config(std::string path, debug_logger dbg = {},
                             std::vector<config_option_t> options = {})
      : path_to_database{std::move(path)},
        options{std::move(options)},
        debug{std::move(dbg)} {}

  bool operator==(const connection_config& other) const {
    return (other.path_to_database == path_to_database &&
            other.options == options);
  }

  bool operator!=(const connection_config& other) const {
    return !operator==(other);
  }

  // Path to the database file, ":memory:" for an anonymous in-memory database.
  std::string path_to_database = ":memory:";

  // Key/value pairs handed to duckdb_config before the database is opened,
  // e.g. {"access_mode", "READ_ONLY"}.
  std::vector<config_option_t> options;

  debug_logger debug;  // not compared
};

}  // namespace sqlpp::duck_db
