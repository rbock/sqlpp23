#pragma once

/**
 * Copyright © 2014-2015, Matthijs Möhlmann
 * Copyright (c) 2023, Vesselin Atanasov
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

#include <memory>
#include "sqlpp23/core/query/statement.h"
#include "sqlpp23/core/type_traits.h"

#include <sqlpp23/core/database/connection.h>
#include <sqlpp23/core/database/transaction.h>
#include <sqlpp23/core/query/statement_constructor_arg.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/postgresql/bind_result.h>
#include <sqlpp23/postgresql/database/connection_config.h>
#include <sqlpp23/postgresql/database/connection_handle.h>
#include <sqlpp23/postgresql/database/exception.h>
#include <sqlpp23/postgresql/database/serializer_context.h>
#include <sqlpp23/postgresql/prepared_statement.h>
#include <sqlpp23/postgresql/result.h>
#include <sqlpp23/postgresql/to_sql_string.h>

struct pg_conn;
typedef struct pg_conn PGconn;

namespace sqlpp::postgresql {

namespace detail {
inline prepared_statement_t prepare_statement(
    connection_handle& handle,
    const std::string& stmt,
    const size_t& param_count) {
  if constexpr (debug_enabled) {
    handle.debug().log(log_category::statement, "preparing: {}", stmt);
  }

  return prepared_statement_t{
      handle.native_handle(), stmt, handle.get_prepared_statement_name(),
      param_count, handle.config.get()};
}

inline Result execute_prepared_statement(
    connection_handle& handle,
    prepared_statement_t& prepared) {
  if constexpr (debug_enabled) {
    handle.debug().log(log_category::statement,
                       "executing prepared statement: {}", prepared.name());
  }
  return prepared.execute();
}
}  // namespace detail

// Base connection class
class connection_base : public sqlpp::connection {
 public:
  using _connection_base_t = connection_base;
  using _config_t = connection_config;
  using _config_ptr_t = std::shared_ptr<const _config_t>;
  using _handle_t = detail::connection_handle;

  using _prepared_statement_t = prepared_statement_t;

 private:
  friend class sqlpp::statement_handler_t;

  bool _transaction_active{false};

  void validate_connection_handle() const {
    if (!_handle.native_handle()) {
      throw std::logic_error{"connection handle used, but not initialized"};
    }
  }

  // direct execution
  Result _execute_impl(
      std::string_view stmt) {
    validate_connection_handle();
    if constexpr (debug_enabled) {
      _handle.debug().log(log_category::statement, "executing: '{}'",
                                 stmt);
    }

    return Result{PQexec(native_handle(), stmt.data())};
  }

  bind_result_t select_impl(const std::string& stmt) {
    return {_execute_impl(stmt), _handle.config.get()};
  }

  size_t insert_impl(const std::string& stmt) {
    return static_cast<size_t>(_execute_impl(stmt).affected_rows());
  }

  size_t update_impl(const std::string& stmt) {
    return static_cast<size_t>(_execute_impl(stmt).affected_rows());
  }

  size_t delete_from_impl(const std::string& stmt) {
    return static_cast<size_t>(_execute_impl(stmt).affected_rows());
  }

  // prepared execution
  prepared_statement_t prepare_impl(const std::string& stmt,
                                    const size_t& param_count) {
    validate_connection_handle();
    return prepare_statement(_handle, stmt, param_count);
  }

  bind_result_t run_prepared_select_impl(prepared_statement_t& prep) {
    validate_connection_handle();
    return {detail::execute_prepared_statement(_handle, prep),
            _handle.config.get()};
  }

  size_t run_prepared_execute_impl(prepared_statement_t& prep) {
    validate_connection_handle();
    Result result = detail::execute_prepared_statement(_handle, prep);
    return static_cast<size_t>(result.affected_rows());
  }

  size_t run_prepared_insert_impl(prepared_statement_t& prep) {
    validate_connection_handle();
    Result result = detail::execute_prepared_statement(_handle, prep);
    return static_cast<size_t>(result.affected_rows());
  }

  size_t run_prepared_update_impl(prepared_statement_t& prep) {
    validate_connection_handle();
    Result result = detail::execute_prepared_statement(_handle, prep);
    return static_cast<size_t>(result.affected_rows());
  }

  size_t run_prepared_delete_from_impl(prepared_statement_t& prep) {
    validate_connection_handle();
    Result result = detail::execute_prepared_statement(_handle, prep);
    return static_cast<size_t>(result.affected_rows());
  }

  // Select stmt (returns a result)
  template <typename Select>
  bind_result_t _select(const Select& s) {
    context_t context(this);
    return select_impl(to_sql_string(context, s));
  }

  // Prepared select
  template <typename Select>
  _prepared_statement_t _prepare_select(const Select& s) {
    context_t context(this);
    return prepare_impl(to_sql_string(context, s), context._count);
  }

  template <typename PreparedSelect>
  bind_result_t _run_prepared_select(PreparedSelect& s) {
    s._bind_params();
    return run_prepared_select_impl(s._prepared_statement);
  }

  // Insert
  template <typename Insert>
  size_t _insert(const Insert& s) {
    context_t context(this);
    return insert_impl(to_sql_string(context, s));
  }

  template <typename Insert>
  prepared_statement_t _prepare_insert(const Insert& s) {
    context_t context(this);
    return prepare_impl(to_sql_string(context, s), context._count);
  }

  template <typename PreparedInsert>
  size_t _run_prepared_insert(PreparedInsert& i) {
    i._bind_params();
    return run_prepared_insert_impl(i._prepared_statement);
  }

  // Update
  template <typename Update>
  size_t _update(const Update& s) {
    context_t context(this);
    return update_impl(to_sql_string(context, s));
  }

  template <typename Update>
  prepared_statement_t _prepare_update(const Update& s) {
    context_t context(this);
    return prepare_impl(to_sql_string(context, s), context._count);
  }

  template <typename PreparedUpdate>
  size_t _run_prepared_update(PreparedUpdate& u) {
    u._bind_params();
    return run_prepared_update_impl(u._prepared_statement);
  }

  // Delete
  template <typename Delete>
  size_t _delete_from(const Delete& s) {
    context_t context(this);
    return delete_from_impl(to_sql_string(context, s));
  }

  template <typename Delete>
  prepared_statement_t _prepare_delete_from(const Delete& s) {
    context_t context(this);
    return prepare_impl(to_sql_string(context, s), context._count);
  }

  template <typename PreparedDelete>
  size_t _run_prepared_delete_from(PreparedDelete& r) {
    r._bind_params();
    return run_prepared_delete_from_impl(r._prepared_statement);
  }

  // Execute
  template <typename Execute>
  size_t _execute(const Execute& s) {
    context_t context(this);
    return operator()(to_sql_string(context, s));
  }

  template <typename Execute>
  _prepared_statement_t _prepare_execute(const Execute& s) {
    context_t context(this);
    return prepare_impl(to_sql_string(context, s), context._count);
  }

  template <typename PreparedExecute>
  size_t _run_prepared_execute(PreparedExecute& x) {
    x._prepared_statement._reset();
    x._bind_params();
    return run_prepared_execute_impl(x._prepared_statement);
  }

 public:
  //! Execute a single statement (like creating a table).
  //! Note that technically, this supports executing multiple statements today,
  //! but this is likely to change to align with other connectors.
  size_t operator()(std::string_view stmt) {
    return static_cast<size_t>(_execute_impl(stmt).affected_rows());
  }

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

  template <typename T>
    requires(sqlpp::is_statement_v<T>)
  auto prepare(const T& t) {
    sqlpp::check_prepare_consistency(t).verify();
    sqlpp::check_compatibility<context_t>(t).verify();
    return sqlpp::statement_handler_t{}.prepare(t, *this);
  }

  //! set the default transaction isolation level to use for new transactions
  void set_default_isolation_level(isolation_level level) {
    std::string level_str = "read uncommmitted";
    switch (level) {
      /// @todo what about undefined ?
      case isolation_level::read_committed:
        level_str = "read committed";
        break;
      case isolation_level::read_uncommitted:
        level_str = "read uncommitted";
        break;
      case isolation_level::repeatable_read:
        level_str = "repeatable read";
        break;
      case isolation_level::serializable:
        level_str = "serializable";
        break;
      default:
        throw sqlpp::exception{"Invalid isolation level"};
    }
    std::string cmd =
        "SET default_transaction_isolation to '" + level_str + "'";
    _execute_impl(cmd);
  }

  //! get the currently set default transaction isolation level
  isolation_level get_default_isolation_level() {
    auto res = _execute_impl("SHOW default_transaction_isolation;");
    auto status = res.status();
    if ((status != PGRES_TUPLES_OK) && (status != PGRES_COMMAND_OK)) {
      throw sqlpp::exception{
          "PostgreSQL error: could not read default_transaction_isolation"};
    }

    auto in = res.get_string_value(0, 0);
    if (in == "read committed") {
      return isolation_level::read_committed;
    } else if (in == "read uncommitted") {
      return isolation_level::read_uncommitted;
    } else if (in == "repeatable read") {
      return isolation_level::repeatable_read;
    } else if (in == "serializable") {
      return isolation_level::serializable;
    }
    return isolation_level::undefined;
  }

  //! create savepoint
  void savepoint(const std::string& name) {
    /// NOTE prevent from sql injection?
    _execute_impl("SAVEPOINT " + name);
  }

  //! ROLLBACK TO SAVEPOINT
  void rollback_to_savepoint(const std::string& name) {
    /// NOTE prevent from sql injection?
    _execute_impl("ROLLBACK TO SAVEPOINT " + name);
  }

  //! release_savepoint
  void release_savepoint(const std::string& name) {
    /// NOTE prevent from sql injection?
    _execute_impl("RELEASE SAVEPOINT " + name);
  }

  //! start transaction
  void start_transaction(isolation_level level = isolation_level::undefined) {
    if (_transaction_active) {
      throw sqlpp::exception{"PostgreSQL error: transaction already open"};
    }
    switch (level) {
      case isolation_level::serializable: {
        _execute_impl("BEGIN ISOLATION LEVEL SERIALIZABLE");
        break;
      }
      case isolation_level::repeatable_read: {
        _execute_impl("BEGIN ISOLATION LEVEL REPEATABLE READ");
        break;
      }
      case isolation_level::read_committed: {
        _execute_impl("BEGIN ISOLATION LEVEL READ COMMITTED");
        break;
      }
      case isolation_level::read_uncommitted: {
        _execute_impl("BEGIN ISOLATION LEVEL READ UNCOMMITTED");
        break;
      }
      case isolation_level::undefined: {
        _execute_impl("BEGIN");
        break;
      }
    }
    _transaction_active = true;
  }

  //! commit transaction (or throw transaction if transaction has finished
  //! already)
  void commit_transaction() {
    if (!_transaction_active) {
      throw sqlpp::exception{
          "PostgreSQL error: transaction failed or finished."};
    }
    _execute_impl("COMMIT");
    _transaction_active = false;
  }

  //! rollback transaction
  void rollback_transaction() {
    if (!_transaction_active) {
      throw sqlpp::exception{
          "PostgreSQL error: transaction failed or finished."};
    }
    if constexpr (debug_enabled) {
      _handle.debug().log(log_category::connection,
                                 "rolling back unfinished transaction");
    }
    _execute_impl("ROLLBACK");
    _transaction_active = false;
  }

  //! report rollback failure
  void report_rollback_failure(const std::string& message) noexcept {
    if constexpr (debug_enabled) {
      _handle.debug().log(log_category::connection,
                                 "transaction rollback failure: {}", message);
    }
  }

  //! check if transaction is active
  bool is_transaction_active() { return _transaction_active; }

  //! get the last inserted id for a certain table
  uint64_t last_insert_id(const std::string& table,
                          const std::string& fieldname) {
    std::string sql = "SELECT currval('" + table + "_" + fieldname + "_seq')";
    PGresult* res = PQexec(native_handle(), sql.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
      std::string err{PQresultErrorMessage(res)};
      PQclear(res);
      throw sqlpp::postgresql::undefined_table{err, sql};
    }

    // Parse the number and return.
    std::string in{PQgetvalue(res, 0, 0)};
    PQclear(res);
    return std::stoul(in);
  }

  ::PGconn* native_handle() const { return _handle.native_handle(); }

  std::string escape(const std::string_view& s) const {
    validate_connection_handle();
    // Escape strings
    std::string result;
    result.resize((s.size() * 2) + 1);

    int err;
    size_t length = PQescapeStringConn(native_handle(), &result[0], s.data(),
                                       s.size(), &err);
    result.resize(length);
    return result;
  }

 protected:
  _handle_t _handle;

  // Constructors
  connection_base() = default;
  connection_base(_handle_t&& handle) : _handle{std::move(handle)} {}
};

inline auto context_t::escape(std::string_view t) -> std::string {
  return _db->escape(t);
}

using connection = sqlpp::normal_connection<connection_base>;
using pooled_connection = sqlpp::pooled_connection<connection_base>;
}  // namespace sqlpp::postgresql

