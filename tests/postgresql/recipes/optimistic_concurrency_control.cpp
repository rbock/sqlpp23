/*
 * Copyright (c) 2025, Vesselin Atanasov
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

// A sample program demonstrating the optimistic concurrency control pattern,
// which allows concurrent access to shared data, while avoiding placing any
// explicit locks on the shared data.
//
// For details on the actual pattern see /docs/recipes/optimistic_concurrency_control.md

#include <sqlpp23/postgresql/postgresql.h>
#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/tests/postgresql/make_test_connection.h>
#include <sqlpp23/tests/postgresql/tables.h>

#include <iostream>
#include <thread>
#include <vector>

namespace sql = ::sqlpp::postgresql;

sql::pooled_connection get_connection() {
  static auto pool = sql::connection_pool{sql::make_test_config(), 5};
  return pool.get();
}

// The "tx" (short for "transaction") function is the actual implementation of the
// optimistic concurrency control pattern. It calls the user-provided handler in a
// loop until the handler succeeds propagation the result from the handler to the
// caller of "tx". On a failure (exception), the error code is checked, retryable
// errors are ignored and the transaction is retried, non-retryable errors bubble
// up to the called of "tx".
//
// For details on the actual pattern see /docs/recipes/optimistic_concurrency_control.md
//
template <typename F>
  requires std::invocable<F, sql::pooled_connection&>
auto tx(F&& handler) {
  auto dbc = get_connection();
  for (;;) {
    dbc.start_transaction(sqlpp::isolation_level::serializable);
    try {
      if constexpr (std::is_void_v<decltype(handler(dbc))>) {
        handler(dbc);
        dbc.commit_transaction();
        return;
      } else {
        auto&& result = handler(dbc);
        dbc.commit_transaction();
        return result;
      }
    } catch (const sql::result_exception& ex) {
      auto ec = ex.sql_state();
      if ((ec == "40001") ||  // Serialization failure
          (ec == "40P01")     // Deadlock detected
      ) {
        // Recoverable transaction error. Retry.
      } else {
        throw;
      }
    }
    dbc.rollback_transaction();
  }
}

void tab_bar_create() {
  auto dbc = get_connection();
  test::createTabBar(dbc);
  auto tb = test::TabBar{};
  dbc(insert_into(tb).set(tb.intN = 0));
}

void tab_bar_update_st(int num_updates) {
  auto tb = test::TabBar{};
  for (int i = 0; i < num_updates; ++i) {
    tx([&](auto&& dbc) {
      auto value = dbc(select(tb.intN).from(tb).limit(1)).front().intN.value();
      ++value;
      dbc(update(tb).set(tb.intN = value));
    });
  }
}

void tab_bar_update_mt(int num_threads, int num_updates) {
  auto threads = std::vector<std::thread>{};
  for (auto i = 0; i < num_threads; ++i) {
    auto func = __func__;
    threads.push_back(std::thread([num_updates, &func]() {
      try {
        tab_bar_update_st(num_updates);
      } catch (const std::exception& e) {
        std::cerr << std::string(func) + ": In-thread exception: " + e.what() +
                         "\n";
        std::abort();
      }
    }));
  }
  for (auto&& t : threads) {
    t.join();
  }
}

void tab_bar_check(int expected) {
  auto dbc = get_connection();
  auto tb = test::TabBar{};
  auto actual = dbc(select(tb.intN).from(tb).limit(1)).front().intN.value();
  if (actual != expected) {
    throw std::runtime_error{
        std::format("Expected value {}, but got {}", expected, actual)};
  }
}

int main() {
  try {
    int num_threads = PQisthreadsafe() ? 2 : 1;
    int num_updates = 50;
    tab_bar_create();
    if (num_threads == 1) {
      tab_bar_update_st(num_updates);
    } else {
      tab_bar_update_mt(num_threads, num_updates);
    }
    tab_bar_check(num_threads * num_updates);
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
