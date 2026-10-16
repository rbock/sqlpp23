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

#include <duckdb.h>

#include <cstdlib>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <sqlpp23/duck_db/database/connection_config.h>
#include <sqlpp23/duck_db/database/exception.h>

namespace sqlpp::duck_db::detail {

struct config_deleter {
  void operator()(::duckdb_config* configuration) const noexcept {
    if (configuration) {
      ::duckdb_destroy_config(configuration);
      delete configuration;
    }
  }
};

// A DuckDB database file can only be opened once per process; a second
// duckdb_open_ext on the same path fails with an IO error (file lock). Since
// several sqlpp23 connections may point at the same database -- most notably
// every connection produced by a connection pool -- database instances are
// shared and reference counted here.
//
// The instance cache key is the path, so all connections to one path go through
// the same duckdb_database. ":memory:" is special: DuckDB treats it as an
// anonymous database, and every connection is expected to get its own private
// one, so it is not deduplicated.
class database_cache {
 public:
  struct instance_entry {
    ::duckdb_database database = nullptr;
    size_t users = 0;
  };

  static database_cache& get() {
    static database_cache cache;
    return cache;
  }

  // Returns a database for `path` and increases its user count by one.
  // `database_options` is only applied when the instance is created.
  ::duckdb_database acquire(
      const std::string& path,
      const std::vector<std::pair<std::string, std::string>>& database_options) {
    std::lock_guard<std::mutex> lock{_mutex};

    if (path != ":memory:") {
      const auto existing = _instances.find(path);
      if (existing != _instances.end()) {
        ++existing->second.users;
        return existing->second.database;
      }
    }

    // Objects owned until the database exists, then either handed over to the
    // cache or released again.
    auto config_handle =
        std::unique_ptr<::duckdb_config, config_deleter>{};
    if (not database_options.empty()) {
      config_handle = std::unique_ptr<::duckdb_config, config_deleter>{
          new ::duckdb_config{nullptr}};
      if (::duckdb_create_config(config_handle.get()) != DuckDBSuccess) {
        throw exception{"DuckDB: cannot create a database configuration",
                        static_cast<int>(DuckDBError)};
      }
      for (const auto& [name, value] : database_options) {
        if (::duckdb_set_config(*config_handle, name.c_str(), value.c_str()) !=
            DuckDBSuccess) {
          throw exception{"DuckDB: unknown configuration option '" + name + "'",
                          static_cast<int>(DuckDBError)};
        }
      }
    }

    ::duckdb_database database = nullptr;
    char* error_message = nullptr;
    const auto rc =
        ::duckdb_open_ext(path.c_str(), &database,
                          config_handle ? *config_handle : nullptr,
                          &error_message);
    if (rc != DuckDBSuccess) {
      const auto message = error_message
                               ? std::string{error_message}
                               : std::string{"DuckDB: cannot open database"};
      throw exception{message, static_cast<int>(rc)};
    }

    if (path == ":memory:") {
      // Private to this connection, not tracked in the cache.
      return database;
    }

    _instances.emplace(path, instance_entry{database, 1});
    return database;
  }

  // Decreases the user count and closes the database when the last user is gone.
  void release(const std::string& path, ::duckdb_database database) {
    if (path == ":memory:") {
      if (database) {
        ::duckdb_close(&database);
      }
      return;
    }

    std::lock_guard<std::mutex> lock{_mutex};
    const auto entry = _instances.find(path);
    if (entry == _instances.end()) {
      return;
    }
    if (--entry->second.users == 0) {
      auto cached_database = entry->second.database;
      _instances.erase(entry);
      if (cached_database) {
        ::duckdb_close(&cached_database);
      }
    }
  }

 private:
  database_cache() = default;

  std::mutex _mutex;
  std::map<std::string, instance_entry> _instances;
};

// A connection to a DuckDB database. The database instance is shared (see
// database_cache), the connection itself belongs to this handle.
struct connection_handle {
  struct connection_deleter {
    void operator()(::duckdb_connection* connection) const noexcept {
      if (connection) {
        ::duckdb_disconnect(connection);
        delete connection;
      }
    }
  };

  using connection_ptr_t =
      std::unique_ptr<::duckdb_connection, connection_deleter>;

  std::shared_ptr<const connection_config> config;
  ::duckdb_database database = nullptr;
  connection_ptr_t connection{};

  connection_handle() = default;

  connection_handle(const std::shared_ptr<const connection_config>&
                        configuration)
      : config{configuration} {
    if (not config) {
      return;
    }

    const auto& path = config->path_to_database;
    database = database_cache::get().acquire(path, config->options);
    try {
      connection_ptr_t new_connection{new ::duckdb_connection{nullptr}};
      if (::duckdb_connect(database, new_connection.get()) != DuckDBSuccess) {
        throw exception{"DuckDB: cannot create a connection to '" + path + "'",
                        static_cast<int>(DuckDBError)};
      }
      connection = std::move(new_connection);
    } catch (...) {
      database_cache::get().release(path, database);
      database = nullptr;
      throw;
    }
  }

  connection_handle(const connection_handle&) = delete;
  connection_handle(connection_handle&& rhs) noexcept
      : config{std::move(rhs.config)},
        database{rhs.database},
        connection{std::move(rhs.connection)} {
    rhs.database = nullptr;
  }
  connection_handle& operator=(const connection_handle&) = delete;
  connection_handle& operator=(connection_handle&& rhs) noexcept {
    if (this != &rhs) {
      release_database();
      config = std::move(rhs.config);
      database = rhs.database;
      connection = std::move(rhs.connection);
      rhs.database = nullptr;
    }
    return *this;
  }

  ~connection_handle() { release_database(); }

  ::duckdb_connection native_handle() const {
    return connection ? *connection : nullptr;
  }

  bool is_connected() const {
    // The connection is established in the constructor. DuckDB does not offer a
    // passive way of checking whether it is still usable. Note that a default
    // constructed handle owns no connection at all.
    return connection and *connection != nullptr;
  }

  bool ping_server() const {
    if (not is_connected()) {
      return false;
    }
    ::duckdb_result result{};
    const auto rc = ::duckdb_query(*connection, "SELECT 1", &result);
    const auto failed = rc != DuckDBSuccess;
    ::duckdb_destroy_result(&result);
    return not failed;
  }

  const debug_logger& debug() const { return config->debug; }

  static std::string library_version() {
    const auto version = ::duckdb_library_version();
    return version ? std::string{version} : std::string{};
  }

 private:
  void release_database() noexcept {
    if (database and config) {
      database_cache::get().release(config->path_to_database, database);
    }
    database = nullptr;
  }
};

}  // namespace sqlpp::duck_db::detail
