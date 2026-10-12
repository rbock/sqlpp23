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

#include <memory>
#include <string>

#include <sqlpp23/core/basic/schema.h>
#include <sqlpp23/core/database/connection.h>
#include <sqlpp23/core/database/transaction.h>
#include <sqlpp23/core/query/statement_handler.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/core/type_traits.h>
#include <sqlpp23/duck_db/bind_result.h>
#include <sqlpp23/duck_db/constraints.h>
#include <sqlpp23/duck_db/database/connection_config.h>
#include <sqlpp23/duck_db/database/connection_handle.h>
#include <sqlpp23/duck_db/database/exception.h>
#include <sqlpp23/duck_db/database/serializer_context.h>
#include <sqlpp23/duck_db/prepared_statement.h>
#include <sqlpp23/duck_db/to_sql_string.h>

namespace sqlpp::duck_db {

namespace detail {
inline prepared_statement_t prepare_statement(connection_handle& handle,
                                              std::string_view statement) {
  return prepared_statement_t{handle.native_handle(), statement,
                              handle.config.get()};
}

// Executes a prepared statement and transfers ownership of the result into a
// result_holder, which keeps the result alive for as long as it is needed.
// The result is heap allocated and never copied or moved: DuckDB's duckdb_result
// is a handle to materialized data, and duplicating the struct would duplicate
// the handle without duplicating the data.
inline result_holder_ptr_t execute_statement(prepared_statement_t& prepared) {
  auto native_result = std::make_unique<::duckdb_result>();
  const auto rc = ::duckdb_execute_prepared(prepared.native_handle(),
                                            native_result.get());
  if (rc != DuckDBSuccess) {
    const auto* error = ::duckdb_result_error(native_result.get());
    throw exception{error ? error : "DuckDB: cannot execute statement",
                    static_cast<int>(rc)};
  }
  return std::make_shared<result_holder>(native_result.release());
}

inline result_holder_ptr_t execute_statement(connection_handle& handle,
                                             prepared_statement_t& prepared) {
  if constexpr (debug_enabled) {
    handle.debug().log(log_category::statement, "DuckDB: executing statement");
  }
  return execute_statement(prepared);
}

inline uint64_t affected_rows(const detail::result_holder_ptr_t& result) {
  return result ? static_cast<uint64_t>(::duckdb_rows_changed(
                      &const_cast<::duckdb_result&>(result->result)))
                : 0;
}

// DuckDB has no last_insert_rowid(). When the statement returns a query result
// with rows (e.g. because of a RETURNING clause), the first column of the first
// row is used as the last insert id. Otherwise the id is unknown and 0 is
// returned.
inline uint64_t first_column_as_id(const result_holder_ptr_t& result) {
  if (not result or result->row_count == 0 or
      ::duckdb_column_count(&result->result) == 0) {
    return 0;
  }
  if (::duckdb_result_return_type(result->result) !=
      DUCKDB_RESULT_TYPE_QUERY_RESULT) {
    // INSERT / UPDATE / DELETE without RETURNING produce a CHANGED_ROWS result
    // whose single column carries the number of affected rows, not an id.
    return 0;
  }
  return static_cast<uint64_t>(::duckdb_value_int64(&result->result, 0, 0));
}
}  // namespace detail

struct command_result {
  uint64_t affected_rows;
};

struct insert_result {
  uint64_t affected_rows;
  uint64_t last_insert_id;
};

// Base connection class
class connection_base : public sqlpp::connection {
 public:
  using _connection_base_t = connection_base;
  using _config_t = connection_config;
  using _config_ptr_t = std::shared_ptr<const connection_config>;
  using _handle_t = detail::connection_handle;

  using _prepared_statement_t = prepared_statement_t;

 private:
  friend sqlpp::statement_handler_t;

  bool _transaction_active{false};

  // direct execution
  command_result execute_impl(std::string_view statement) {
    auto prepared = detail::prepare_statement(_handle, statement);
    const auto result = detail::execute_statement(_handle, prepared);

    return {.affected_rows = detail::affected_rows(result)};
  }

  bind_result_t select_impl(const std::string& statement) {
    auto prepared = detail::prepare_statement(_handle, statement);
    auto result = detail::execute_statement(_handle, prepared);

    return {native_handle(), std::move(result), _handle.config.get()};
  }

  insert_result insert_impl(const std::string& statement) {
    auto prepared = detail::prepare_statement(_handle, statement);
    auto result = detail::execute_statement(_handle, prepared);

    _last_insert_id = detail::first_column_as_id(result);
    return {
        .affected_rows = detail::affected_rows(result),
        .last_insert_id = _last_insert_id};
  }

  command_result update_impl(const std::string& statement) {
    auto prepared = detail::prepare_statement(_handle, statement);
    const auto result = detail::execute_statement(_handle, prepared);
    return {.affected_rows = detail::affected_rows(result)};
  }

  command_result delete_from_impl(const std::string& statement) {
    auto prepared = detail::prepare_statement(_handle, statement);
    const auto result = detail::execute_statement(_handle, prepared);
    return {.affected_rows = detail::affected_rows(result)};
  }

  // prepared execution
  prepared_statement_t prepare_impl(const std::string& statement) {
    return detail::prepare_statement(_handle, statement);
  }

  bind_result_t run_prepared_select_impl(
      prepared_statement_t& prepared_statement) {
    auto result = detail::execute_statement(_handle, prepared_statement);
    return {native_handle(), std::move(result), prepared_statement.config};
  }

  insert_result run_prepared_insert_impl(
      prepared_statement_t& prepared_statement) {
    auto result = detail::execute_statement(_handle, prepared_statement);

    _last_insert_id = detail::first_column_as_id(result);
    return {
        .affected_rows = detail::affected_rows(result),
        .last_insert_id = _last_insert_id};
  }

  command_result run_prepared_update_impl(
      prepared_statement_t& prepared_statement) {
    const auto result = detail::execute_statement(_handle, prepared_statement);

    return {.affected_rows = detail::affected_rows(result)};
  }

  command_result run_prepared_delete_from_impl(
      prepared_statement_t& prepared_statement) {
    const auto result = detail::execute_statement(_handle, prepared_statement);

    return {.affected_rows = detail::affected_rows(result)};
  }

  command_result run_prepared_execute_impl(
      prepared_statement_t& prepared_statement) {
    const auto result = detail::execute_statement(_handle, prepared_statement);

    return {.affected_rows = detail::affected_rows(result)};
  }

  //! select returns a result (which can be iterated row by row)
  template <typename Select>
  bind_result_t _select(const Select& s) {
    context_t context{this};
    auto query = to_sql_string(context, s);
    return select_impl(query);
  }

  template <typename Select>
  _prepared_statement_t _prepare_select(const Select& s) {
    context_t context{this};
    auto query = to_sql_string(context, s);
    return prepare_impl(query);
  }

  template <typename PreparedSelect>
  bind_result_t _run_prepared_select(PreparedSelect& s) {
    sqlpp::statement_handler_t{}.get_prepared_statement(s)._reset();
    sqlpp::statement_handler_t{}.bind_parameters(s);
    return run_prepared_select_impl(
        sqlpp::statement_handler_t{}.get_prepared_statement(s));
  }

  //! insert returns the last auto_incremented id (or zero, if there is none)
  template <typename Insert>
  insert_result _insert(const Insert& i) {
    context_t context{this};
    auto query = to_sql_string(context, i);
    return insert_impl(query);
  }

  template <typename Insert>
  _prepared_statement_t _prepare_insert(const Insert& i) {
    context_t context{this};
    auto query = to_sql_string(context, i);
    return prepare_impl(query);
  }

  template <typename PreparedInsert>
  insert_result _run_prepared_insert(PreparedInsert& i) {
    sqlpp::statement_handler_t{}.get_prepared_statement(i)._reset();
    sqlpp::statement_handler_t{}.bind_parameters(i);
    return run_prepared_insert_impl(
        sqlpp::statement_handler_t{}.get_prepared_statement(i));
  }

  //! update returns the number of affected rows
  template <typename Update>
  command_result _update(const Update& u) {
    context_t context{this};
    auto query = to_sql_string(context, u);
    return update_impl(query);
  }

  template <typename Update>
  _prepared_statement_t _prepare_update(const Update& u) {
    context_t context{this};
    auto query = to_sql_string(context, u);
    return prepare_impl(query);
  }

  template <typename PreparedUpdate>
  command_result _run_prepared_update(PreparedUpdate& u) {
    sqlpp::statement_handler_t{}.get_prepared_statement(u)._reset();
    sqlpp::statement_handler_t{}.bind_parameters(u);
    return run_prepared_update_impl(
        sqlpp::statement_handler_t{}.get_prepared_statement(u));
  }

  //! delete_from returns the number of deleted rows
  template <typename Delete>
  command_result _delete_from(const Delete& r) {
    context_t context{this};
    auto query = to_sql_string(context, r);
    return delete_from_impl(query);
  }

  template <typename Delete>
  _prepared_statement_t _prepare_delete_from(const Delete& r) {
    context_t context{this};
    auto query = to_sql_string(context, r);
    return prepare_impl(query);
  }

  template <typename PreparedDelete>
  command_result _run_prepared_delete_from(PreparedDelete& r) {
    sqlpp::statement_handler_t{}.get_prepared_statement(r)._reset();
    sqlpp::statement_handler_t{}.bind_parameters(r);
    return run_prepared_delete_from_impl(
        sqlpp::statement_handler_t{}.get_prepared_statement(r));
  }

  //! Execute a single arbitrary statement (e.g. create a table)
  //! Throws an exception if multiple statements are passed (e.g. separated by
  //! semicolon).
  template <typename Execute>
  command_result _execute(const Execute& r) {
    context_t context{this};
    auto query = to_sql_string(context, r);
    return execute_impl(query);
  }

  template <typename Execute>
  _prepared_statement_t _prepare_execute(const Execute& x) {
    context_t context{this};
    auto query = to_sql_string(context, x);
    return prepare_impl(query);
  }

  template <typename PreparedExecute>
  command_result _run_prepared_execute(PreparedExecute& x) {
    sqlpp::statement_handler_t{}.get_prepared_statement(x)._reset();
    sqlpp::statement_handler_t{}.bind_parameters(x);
    return run_prepared_execute_impl(
        sqlpp::statement_handler_t{}.get_prepared_statement(x));
  }

 public:
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

  command_result operator()(std::string_view t) { return execute_impl(t); }

  template <typename T>
    requires(sqlpp::is_statement_v<T>)
  auto prepare(const T& t) {
    sqlpp::check_prepare_consistency(t).verify();
    sqlpp::check_compatibility<context_t>(t).verify();
    return sqlpp::statement_handler_t{}.prepare(t, *this);
  }

  //! DuckDB uses snapshot isolation for all connections and does not expose a
  //! per-connection isolation level through the C API.
  //! Setting the default isolation level is therefore only accepted for the
  //! values that match DuckDB's behaviour.
  void set_default_isolation_level(isolation_level level) {
    if (level == isolation_level::serializable or
        level == isolation_level::undefined) {
      return;
    }
    throw sqlpp::exception{
        "DuckDB: setting the default isolation level is not supported, DuckDB "
        "uses snapshot isolation"};
  }

  isolation_level get_default_isolation_level() {
    return isolation_level::serializable;
  }

  //! start transaction
  void start_transaction() {
    auto prepared = detail::prepare_statement(_handle, "BEGIN TRANSACTION");
    detail::execute_statement(_handle, prepared);
    _transaction_active = true;
  }

  //! commit transaction
  void commit_transaction() {
    auto prepared = detail::prepare_statement(_handle, "COMMIT");
    detail::execute_statement(_handle, prepared);
    _transaction_active = false;
  }

  //! rollback transaction with or without reporting the rollback
  void rollback_transaction() {
    if constexpr (debug_enabled) {
      _handle.debug().log(
          log_category::connection,
          "DuckDB warning: Rolling back unfinished transaction");
    }
    auto prepared = detail::prepare_statement(_handle, "ROLLBACK");
    detail::execute_statement(_handle, prepared);
    _transaction_active = false;
  }

  //! report a rollback failure (will be called by transactions in case of a
  //! rollback failure in the destructor)
  void report_rollback_failure(const std::string& message) noexcept {
    if constexpr (debug_enabled) {
      _handle.debug().log(log_category::connection, "rollback failure: ",
                          message);
    }
  }

  //! check if transaction is active
  bool is_transaction_active() { return _transaction_active; }

  //! DuckDB does not know about last_insert_rowid(). This returns the value that
  //! the most recent insert produced through a RETURNING clause, otherwise 0.
  uint64_t last_insert_id() noexcept { return _last_insert_id; }

  ::duckdb_connection native_handle() const { return _handle.native_handle(); }

  //! Attach another database file, e.g. ATTACH 'other.db' AS other
  schema_t attach(const connection_config& config, const std::string& name) {
    context_t context{this};
    auto prepared = detail::prepare_statement(
        _handle, "ATTACH " +
                     sqlpp::to_sql_string(context, config.path_to_database) +
                     " AS " + sqlpp::quoted_name_to_sql_string(context, name));
    detail::execute_statement(_handle, prepared);

    return {name};
  }

  //! Cancel the currently running query on this connection.
  void interrupt() { ::duckdb_interrupt(native_handle()); }

  std::string escape(const std::string_view& s) const {
    auto result = std::string{};
    result.reserve(s.size() * 2);
    for (const auto c : s) {
      if (c == '\'')
        result.push_back(c);  // Escaping
      result.push_back(c);
    }
    return result;
  }

  //! The version of the DuckDB library this connection is linked against.
  static std::string library_version() {
    return detail::connection_handle::library_version();
  }

 protected:
  _handle_t _handle;
  uint64_t _last_insert_id = 0;

  // Constructors
  connection_base() = default;
  connection_base(_handle_t handle) : _handle{std::move(handle)} {}
};

inline auto context_t::escape(std::string_view t) -> std::string {
  return _db->escape(t);
}

using connection = sqlpp::normal_connection<connection_base>;
using pooled_connection = sqlpp::pooled_connection<connection_base>;
}  // namespace sqlpp::duck_db
