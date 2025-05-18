#pragma once

/*
 * Copyright (c) 2013-2015, Roland Bock
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

#include <sqlpp23/core/basic/schema.h>
#include <sqlpp23/core/database/connection.h>
#include <sqlpp23/core/database/transaction.h>
#include <sqlpp23/core/query/result_row.h>
#include <sqlpp23/core/query/statement.h>
#include <sqlpp23/core/query/statement_handler.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/core/type_traits.h>
#include <sqlpp23/mock_db/text_result.h>
#include <sqlpp23/mock_db/database/connection_config.h>
#include <sqlpp23/mock_db/database/connection_handle.h>
#include <sqlpp23/mock_db/database/serializer_context.h>

// an object to store internal Mock flags and values to validate in tests
namespace sqlpp::mock_db {
namespace detail {
struct IsolationMockData {
  sqlpp::isolation_level _last_isolation_level;
  sqlpp::isolation_level _default_isolation_level;
};
}

struct command_result {
  uint64_t affected_rows;
};

struct insert_result {
  uint64_t affected_rows;
  uint64_t last_insert_id;
};

class connection_base : public sqlpp::connection {
 public:
  using _config_t = connection_config;
  using _config_ptr_t = std::shared_ptr<const _config_t>;
  using _handle_t = detail::connection_handle;

  // Directly executed statements start here
  template <typename T>
    requires(sqlpp::is_statement_v<T>)
  auto operator()(const T& t) {
    sqlpp::check_run_consistency(t).verify();
    sqlpp::check_compatibility<context_t>(t).verify();
    return sqlpp::statement_handler_t{}.run(t, *this);
  }

  template <typename T>
    requires(sqlpp::is_prepared_statement_v<std::decay_t<T>>)
  auto operator()(T&& t) {
    return sqlpp::statement_handler_t{}.run(std::forward<T>(t), *this);
  }

  auto operator()(std::string_view) { return command_result{}; }

  template <typename Statement>
  command_result _execute(const Statement& x) {
    context_t context;
    const auto query = to_sql_string(context, x);
    std::cout << "Running execute call with\n" << query << std::endl;
    return {};
  }

  template <typename Insert>
  insert_result _insert(const Insert& x) {
    context_t context;
    const auto query = to_sql_string(context, x);
    std::cout << "Running insert call with\n" << query << std::endl;
    return {};
  }

  template <typename Update>
  command_result _update(const Update& x) {
    context_t context;
    const auto query = to_sql_string(context, x);
    std::cout << "Running update call with\n" << query << std::endl;
    return {};
  }

  template <typename Delete>
  command_result _delete_from(const Delete& x) {
    context_t context;
    const auto query = to_sql_string(context, x);
    std::cout << "Running remove call with\n" << query << std::endl;
    return {};
  }

  template <typename Select>
  text_result_t _select(const Select& x) {
    context_t context;
    const auto query = to_sql_string(context, x);
    std::cout << "Running select call with\n" << query << std::endl;
    return {&_mock_result_data, _handle.config.get()};
  }

  // Prepared statements start here
  using _prepared_statement_t = std::nullptr_t;

  template <typename T>
    requires(sqlpp::is_statement_v<T>)
  auto prepare(const T& t) {
    sqlpp::check_prepare_consistency(t).verify();
    sqlpp::check_compatibility<context_t>(t).verify();
    return sqlpp::statement_handler_t{}.prepare(t, *this);
  }

  template <typename Statement>
  _prepared_statement_t _prepare_execute(const Statement& x) {
    context_t context;
    const auto query = to_sql_string(context, x);
    std::cout << "Running prepare execute call with\n" << query << std::endl;
    return nullptr;
  }

  template <typename Insert>
  _prepared_statement_t _prepare_insert(const Insert& x) {
    context_t context;
    const auto query = to_sql_string(context, x);
    std::cout << "Running prepare insert call with\n" << query << std::endl;
    return nullptr;
  }

  template <typename Update>
  _prepared_statement_t _prepare_update(const Update& x) {
    context_t context;
    const auto query = to_sql_string(context, x);
    std::cout << "Running prepare update call with\n" << query << std::endl;
    return nullptr;
  }

  template <typename PreparedExecute>
  command_result _run_prepared_execute(PreparedExecute&) {
    return {};
  }

  template <typename PreparedInsert>
  insert_result _run_prepared_insert(PreparedInsert&) {
    return {};
  }

  template <typename PreparedUpdate>
  command_result _run_prepared_update(PreparedUpdate&) {
    return {};
  }

  template <typename Select>
  _prepared_statement_t _prepare_select(const Select& x) {
    context_t context;
    const auto query = to_sql_string(context, x);
    std::cout << "Running prepare select call with\n" << query << std::endl;
    return nullptr;
  }

  template <typename PreparedSelect>
  text_result_t _run_prepared_select(PreparedSelect&) {
    return {&_mock_result_data, _handle.config.get()};
  }

  auto attach(std::string name) -> ::sqlpp::schema_t { return {name}; }

  void start_transaction() {
    _mock_data._last_isolation_level = _mock_data._default_isolation_level;
  }

  void start_transaction(sqlpp::isolation_level level) {
    _mock_data._last_isolation_level = level;
  }

  void set_default_isolation_level(sqlpp::isolation_level level) {
    _mock_data._default_isolation_level = level;
  }

  sqlpp::isolation_level get_default_isolation_level() {
    return _mock_data._default_isolation_level;
  }

  void rollback_transaction() {}

  void commit_transaction() {}

  void report_rollback_failure(std::string) {}

  // temporary data store to verify the expected results were produced
  detail::IsolationMockData _mock_data;
  MockRes _mock_result_data;

 protected:
  _handle_t _handle;

  // Constructors
  connection_base() = default;
  connection_base(_handle_t handle) : _handle{std::move(handle)} {}
};

using connection = sqlpp::normal_connection<connection_base>;
using pooled_connection = sqlpp::pooled_connection<connection_base>;

}  // namespace sqlpp::mock_db
