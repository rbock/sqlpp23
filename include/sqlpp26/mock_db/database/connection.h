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

#include <sqlpp23/core/basic/schema.h>
#include <sqlpp23/core/database/connection.h>
#include <sqlpp23/core/database/transaction.h>
#include <sqlpp23/core/query/result_row.h>
#include <sqlpp23/core/query/statement.h>
#include <sqlpp23/core/query/statement_handler.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/core/type_traits.h>
#include <sqlpp23/mock_db/text_result.h>
#include <sqlpp23/mock_db/prepared_statement.h>
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

inline void execute_statement(detail::connection_handle& handle,
                              std::string_view statement) {
  if constexpr (debug_enabled) {
    handle.debug().log(log_category::statement, "Executing: '{}'", statement);
  }
}

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
    execute_statement(_handle, query);
    return {};
  }

  template <typename Insert>
  insert_result _insert(const Insert& x) {
    context_t context;
    const auto query = to_sql_string(context, x);
    execute_statement(_handle, query);
    return {};
  }

  template <typename Update>
  command_result _update(const Update& x) {
    context_t context;
    const auto query = to_sql_string(context, x);
    execute_statement(_handle, query);
    return {};
  }

  template <typename Delete>
  command_result _delete_from(const Delete& x) {
    context_t context;
    const auto query = to_sql_string(context, x);
    execute_statement(_handle, query);
    return {};
  }

  template <typename Select>
  text_result_t _select(const Select& x) {
    context_t context;
    const auto query = to_sql_string(context, x);
    execute_statement(_handle, query);
    return {&_mock_result_data, _handle.config.get()};
  }

  prepared_statement_t prepare_impl(const std::string& statement,
                                    size_t /*no_of_parameters*/) {
    if constexpr (debug_enabled) {
      _handle.debug().log(log_category::statement, "Preparing: '{}'",
                          statement);
    }

    return prepared_statement_t{_handle.config.get()};
  }

  command_result run_prepared_delete_from_impl(
      prepared_statement_t& /*prepared_statement*/) {
    return {.affected_rows = 0};
  }

  command_result run_prepared_execute_impl(
      prepared_statement_t& /*prepared_statement*/) {
    return {.affected_rows = 0};
  }

  insert_result run_prepared_insert_impl(
      prepared_statement_t& /*prepared_statement*/) {
    return {.affected_rows = 0, .last_insert_id = 0};
  }

  text_result_t run_prepared_select_impl(
      prepared_statement_t& /* prepared_statement */,
      size_t /*no_of_columns*/) {
    return {&_mock_result_data, _handle.config.get()};
  }

  command_result run_prepared_update_impl(
      prepared_statement_t& /*prepared_statement*/) {
    return {.affected_rows = 0};
  }

  // Prepared statements start here
  using _prepared_statement_t = prepared_statement_t;

  template <typename T>
    requires(sqlpp::is_statement_v<T>)
  auto prepare(const T& t) {
    sqlpp::check_prepare_consistency(t).verify();
    sqlpp::check_compatibility<context_t>(t).verify();
    return sqlpp::statement_handler_t{}.prepare(t, *this);
  }

  template <typename DeleteFrom>
  _prepared_statement_t _prepare_delete_from(const DeleteFrom& x) {
    context_t context;
    const auto query = to_sql_string(context, x);
    return prepare_impl(query,
                        parameters_of_t<std::decay_t<DeleteFrom>>::size());
  }

  template <typename Statement>
  _prepared_statement_t _prepare_execute(const Statement& x) {
    context_t context;
    const auto query = to_sql_string(context, x);
    return prepare_impl(query,
                        parameters_of_t<std::decay_t<Statement>>::size());
  }

  template <typename Insert>
  _prepared_statement_t _prepare_insert(const Insert& x) {
    context_t context;
    const auto query = to_sql_string(context, x);
    return prepare_impl(query, parameters_of_t<std::decay_t<Insert>>::size());
  }

  template <typename Select>
  _prepared_statement_t _prepare_select(const Select& x) {
    context_t context;
    const auto query = to_sql_string(context, x);
    return prepare_impl(query, parameters_of_t<std::decay_t<Select>>::size());
  }

  template <typename Update>
  _prepared_statement_t _prepare_update(const Update& x) {
    context_t context;
    const auto query = to_sql_string(context, x);
    return prepare_impl(query, parameters_of_t<std::decay_t<Update>>::size());
  }

  template <typename PreparedDelete>
  command_result _run_prepared_delete_from(PreparedDelete& d) {
    statement_handler_t{}.bind_parameters(d);
    return run_prepared_delete_from_impl(
        sqlpp::statement_handler_t{}.get_prepared_statement(d));
  }

  template <typename PreparedExecute>
  command_result _run_prepared_execute(PreparedExecute& e) {
    statement_handler_t{}.bind_parameters(e);
    return run_prepared_execute_impl(
        sqlpp::statement_handler_t{}.get_prepared_statement(e));
  }

  template <typename PreparedInsert>
  insert_result _run_prepared_insert(PreparedInsert& i) {
    statement_handler_t{}.bind_parameters(i);
    return run_prepared_insert_impl(
        sqlpp::statement_handler_t{}.get_prepared_statement(i));
  }

  template <typename PreparedSelect>
  text_result_t _run_prepared_select(PreparedSelect& s) {
    statement_handler_t{}.bind_parameters(s);
    return run_prepared_select_impl(
        sqlpp::statement_handler_t{}.get_prepared_statement(s),
        sqlpp::no_of_result_columns<std::decay_t<PreparedSelect>>::value);
  }

  template <typename PreparedUpdate>
  command_result _run_prepared_update(PreparedUpdate& u) {
    statement_handler_t{}.bind_parameters(u);
    return run_prepared_update_impl(
        sqlpp::statement_handler_t{}.get_prepared_statement(u));
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
