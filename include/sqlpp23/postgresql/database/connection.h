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
#include <sqlpp23/core/query/statement.h>
#include <sqlpp23/core/type_traits.h>

#include <sqlpp23/core/database/connection.h>
#include <sqlpp23/core/database/exception.h>
#include <sqlpp23/core/database/transaction.h>
#include <sqlpp23/core/query/statement_constructor_arg.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/postgresql/database/connection_config.h>
#include <sqlpp23/postgresql/database/connection_handle.h>
#include <sqlpp23/postgresql/database/serializer_context.h>
#include <sqlpp23/postgresql/pg_result.h>
#include <sqlpp23/postgresql/prepared_statement.h>
#include <sqlpp23/postgresql/text_result.h>
#include <sqlpp23/postgresql/to_sql_string.h>
#include <sqlpp23/postgresql/constraints.h>

struct pg_conn;
typedef struct pg_conn PGconn;

namespace sqlpp::postgresql {

struct command_result {
  uint64_t affected_rows;
};

namespace detail {
inline prepared_statement_t prepare_statement(connection_handle& handle,
                                              const std::string& stmt,
                                              const size_t& param_count) {
  if constexpr (debug_enabled) {
    handle.debug().log(log_category::statement, "preparing: {}", stmt);
  }

  return prepared_statement_t{handle.native_handle(), stmt,
                              handle.get_prepared_statement_name(), param_count,
                              handle.config.get()};
}

inline pg_result_t execute_prepared_statement(connection_handle& handle,
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
  pg_result_t _execute_impl(std::string_view stmt) {
    validate_connection_handle();
    if constexpr (debug_enabled) {
      _handle.debug().log(log_category::statement, "executing: '{}'", stmt);
    }

    return pg_result_t{PQexec(native_handle(), stmt.data())};
  }

  text_result_t select_impl(const std::string& stmt) {
    return {_execute_impl(stmt), _handle.config.get()};
  }

  command_result insert_impl(const std::string& stmt) {
    return {.affected_rows = _execute_impl(stmt).affected_rows()};
  }

  command_result update_impl(const std::string& stmt) {
    return {.affected_rows = _execute_impl(stmt).affected_rows()};
  }

  command_result delete_from_impl(const std::string& stmt) {
    return {.affected_rows = _execute_impl(stmt).affected_rows()};
  }

  // prepared execution
  prepared_statement_t prepare_impl(const std::string& stmt,
                                    const size_t& param_count) {
    validate_connection_handle();
    return prepare_statement(_handle, stmt, param_count);
  }

  text_result_t run_prepared_select_impl(prepared_statement_t& prep) {
    validate_connection_handle();
    return {detail::execute_prepared_statement(_handle, prep),
            _handle.config.get()};
  }

  command_result run_prepared_execute_impl(prepared_statement_t& prep) {
    validate_connection_handle();
    pg_result_t result = detail::execute_prepared_statement(_handle, prep);
    return {.affected_rows = result.affected_rows()};
  }

  command_result run_prepared_insert_impl(prepared_statement_t& prep) {
    validate_connection_handle();
    pg_result_t result = detail::execute_prepared_statement(_handle, prep);
    return {.affected_rows = result.affected_rows()};
  }

  command_result run_prepared_update_impl(prepared_statement_t& prep) {
    validate_connection_handle();
    pg_result_t result = detail::execute_prepared_statement(_handle, prep);
    return {.affected_rows = result.affected_rows()};
  }

  command_result run_prepared_delete_from_impl(prepared_statement_t& prep) {
    validate_connection_handle();
    pg_result_t result = detail::execute_prepared_statement(_handle, prep);
    return {.affected_rows = result.affected_rows()};
  }

  // Select stmt (returns a result)
  template <typename Select>
  text_result_t _select(const Select& s) {
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
  text_result_t _run_prepared_select(PreparedSelect& s) {
    sqlpp::statement_handler_t{}.bind_parameters(s);
    return run_prepared_select_impl(
        sqlpp::statement_handler_t{}.get_prepared_statement(s));
  }

  // Insert
  template <typename Insert>
  command_result _insert(const Insert& s) {
    context_t context(this);
    return insert_impl(to_sql_string(context, s));
  }

  template <typename Insert>
  prepared_statement_t _prepare_insert(const Insert& s) {
    context_t context(this);
    return prepare_impl(to_sql_string(context, s), context._count);
  }

  template <typename PreparedInsert>
  command_result _run_prepared_insert(PreparedInsert& i) {
    sqlpp::statement_handler_t{}.bind_parameters(i);
    return run_prepared_insert_impl(
        sqlpp::statement_handler_t{}.get_prepared_statement(i));
  }

  // Update
  template <typename Update>
  command_result _update(const Update& s) {
    context_t context(this);
    return update_impl(to_sql_string(context, s));
  }

  template <typename Update>
  prepared_statement_t _prepare_update(const Update& s) {
    context_t context(this);
    return prepare_impl(to_sql_string(context, s), context._count);
  }

  template <typename PreparedUpdate>
  command_result _run_prepared_update(PreparedUpdate& u) {
    sqlpp::statement_handler_t{}.bind_parameters(u);
    return run_prepared_update_impl(
        sqlpp::statement_handler_t{}.get_prepared_statement(u));
  }

  // Delete
  template <typename Delete>
  command_result _delete_from(const Delete& s) {
    context_t context(this);
    return delete_from_impl(to_sql_string(context, s));
  }

  template <typename Delete>
  prepared_statement_t _prepare_delete_from(const Delete& s) {
    context_t context(this);
    return prepare_impl(to_sql_string(context, s), context._count);
  }

  template <typename PreparedDelete>
  command_result _run_prepared_delete_from(PreparedDelete& r) {
    sqlpp::statement_handler_t{}.bind_parameters(r);
    return run_prepared_delete_from_impl(
        sqlpp::statement_handler_t{}.get_prepared_statement(r));
  }

  // Execute
  template <typename Execute>
  command_result _execute(const Execute& s) {
    context_t context(this);
    return operator()(to_sql_string(context, s));
  }

  template <typename Execute>
  _prepared_statement_t _prepare_execute(const Execute& s) {
    context_t context(this);
    return prepare_impl(to_sql_string(context, s), context._count);
  }

  template <typename PreparedExecute>
  command_result _run_prepared_execute(PreparedExecute& x) {
    sqlpp::statement_handler_t{}.get_prepared_statement(x)._reset();
    sqlpp::statement_handler_t{}.bind_parameters(x);
    return run_prepared_execute_impl(
        sqlpp::statement_handler_t{}.get_prepared_statement(x));
  }

 public:
  //! Execute a single statement (like creating a table).
  //! Note that technically, this supports executing multiple statements today,
  //! but this is likely to change to align with other connectors.
  command_result operator()(std::string_view stmt) {
    return {_execute_impl(stmt).affected_rows()};
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
    auto pg_result = _execute_impl("SHOW default_transaction_isolation;");

    std::string_view in = PQgetvalue(pg_result.get(), 0, 0);
    if (in == std::string_view("read committed")) {
      return isolation_level::read_committed;
    } else if (in == std::string_view("read uncommitted")) {
      return isolation_level::read_uncommitted;
    } else if (in == std::string_view("repeatable read")) {
      return isolation_level::repeatable_read;
    } else if (in == std::string_view("serializable")) {
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

  //! commit transaction
  void commit_transaction() {
    _execute_impl("COMMIT");
    _transaction_active = false;
  }

  //! rollback transaction
  void rollback_transaction() {
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
    auto result = _execute_impl("SELECT currval('" + table + "_" + fieldname + "_seq')");

    // Parse the number and return.
    std::string in{PQgetvalue(result.get(), 0, 0)};
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
  connection_base(_handle_t handle) : _handle{std::move(handle)} {}
};

inline auto context_t::escape(std::string_view t) -> std::string {
  return _db->escape(t);
}

using connection = sqlpp::normal_connection<connection_base>;
using pooled_connection = sqlpp::pooled_connection<connection_base>;
}  // namespace sqlpp::postgresql
