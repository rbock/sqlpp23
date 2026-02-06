/*
 * Copyright (c) 2023-2026, Vesselin Atanasov
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

// A sample program demonstrating how to create and use a thread-safe connection
// using a lazy database connection, connection pool and thread-local storage.
//
// For details on the actual pattern see
// /docs/recipes/thread_local_connection.md

#include <optional>
#include <thread>
#include <vector>

#include <sqlpp23/tests/postgresql/all.h>

namespace sql = ::sqlpp::postgresql;

// This is our main database connection class. It mimics a regular database
// connection, while delegating the execution of SQL queries to an underlying
// sqlpp23 database connection. This underlying database connections is not
// created immediately upon construction of our lazy connection. Instead the
// underlying database connection is created the first time when the user
// tries to execute a database query through operator().
//
// The class constructor received a reference to a connection pool, which later
// is used to get the underlying database connection. When the constructor is
// called, the connection pool does not have to be fully initialized, because
// the constructor does not use the connection pool and just stores the
// reference to it for later use.
//
class lazy_connection {
 private:
  sql::connection_pool& _pool;
  std::optional<sql::pooled_connection> _dbc;

 public:
  lazy_connection(sql::connection_pool& pool) : _pool{pool}, _dbc{} {}
  lazy_connection(const lazy_connection&) = delete;
  lazy_connection(lazy_connection&&) = delete;

  lazy_connection& operator=(const lazy_connection&) = delete;
  lazy_connection& operator=(lazy_connection&&) = delete;

  // Delegate to _dbc any methods of sql::connection that you may need
  // In our example the only delegated method is operator()

  template <typename T>
  auto operator()(const T& t) {
    if (!_dbc) {
      _dbc = _pool.get();
    }
    return (*_dbc)(t);
  }
};

sql::connection_pool g_pool{};

// This is our lazy connection object, which we use to execute SQL queries.
// It is marked with the thread_local storage class specifier, which means
// that the C++ runtime creates one instance of the object per thread, each
// instance having its own underlying database connection.
//
// We don't really care about the order in which the connection pool and
// the global connection object are initialized because, as described above,
// the constructor of the lazy connection only stores a reference to the pool
// without actually using it.
//
thread_local lazy_connection g_dbc{g_pool};

int main() {
  const int num_threads = 5;
  const int num_queries = 10;

  // Initialize the global connection pool
  g_pool.initialize(sql::make_test_config(), num_threads);

  // Spawn the threads and make each thread execute multiple SQL queries
  std::vector<std::thread> threads{};
  for (int i = 0; i < num_threads; ++i) {
    threads.push_back(std::thread([&]() {
      for (int j = 0; j < num_queries; ++j) {
        // For simplicity we don't begin/commit a transaction explicitly.
        // Instead we use the database autocommit mode in which the database
        // engine wraps each quety in its own transaction.
        //
        g_dbc(select(sqlpp::value(1).as(sqlpp::alias::a)));
      }
    }));
  }
  for (auto&& t : threads) {
    t.join();
  }

  return 0;
}
