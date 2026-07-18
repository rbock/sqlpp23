#pragma once

/*
 * Copyright (c) 2026, Leander Schulten
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
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
#include <string>
#include <string_view>

#include <sql.h>
#include <sqlext.h>

#include <sqlpp23/core/database/connection.h>
#include <sqlpp23/core/database/transaction.h>
#include <sqlpp23/core/query/statement_handler.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/core/type_traits.h>
#include <sqlpp23/odbc/cursor_result.h>
#include <sqlpp23/odbc/database/connection_config.h>
#include <sqlpp23/odbc/database/connection_handle.h>
#include <sqlpp23/odbc/database/exception.h>
#include <sqlpp23/odbc/database/serializer_context.h>
#include <sqlpp23/odbc/detail/diagnostics.h>
#include <sqlpp23/odbc/prepared_statement.h>
#include <sqlpp23/odbc/to_sql_string.h>

namespace sqlpp::odbc {

struct command_result {
  uint64_t affected_rows;
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

  void validate_connection_handle() const {
    if (not _handle.native_handle()) {
      throw std::logic_error{"connection handle used, but not initialized"};
    }
  }

  std::shared_ptr<void> allocate_statement() {
    validate_connection_handle();
    SQLHANDLE statement{SQL_NULL_HANDLE};
    const auto rc =
        SQLAllocHandle(SQL_HANDLE_STMT, native_handle(), &statement);
    if (rc != SQL_SUCCESS and rc != SQL_SUCCESS_WITH_INFO) {
      throw detail::make_exception("ODBC: could not allocate statement handle",
                                   SQL_HANDLE_DBC, native_handle());
    }
    return {statement, detail::handle_deleter{SQL_HANDLE_STMT}};
  }

  static uint64_t affected_row_count(SQLHSTMT statement) {
    SQLLEN row_count{0};
    detail::throw_on_error(SQLRowCount(statement, &row_count),
                           "ODBC: could not determine affected row count",
                           SQL_HANDLE_STMT, statement);
    // Drivers report -1 when the count is not applicable.
    return row_count > 0 ? static_cast<uint64_t>(row_count) : 0;
  }

  // direct execution
  command_result execute_direct(std::string_view statement) {
    if constexpr (debug_enabled) {
      _handle.debug().log(log_category::statement, "ODBC: executing: '{}'",
                          statement);
    }
    auto handle = allocate_statement();
    const auto rc = SQLExecDirect(
        handle.get(),
        reinterpret_cast<SQLCHAR*>(const_cast<char*>(statement.data())),
        static_cast<SQLINTEGER>(statement.size()));
    if (rc == SQL_NO_DATA) {
      // Valid outcome of e.g. a searched update affecting no rows.
      return {.affected_rows = 0};
    }
    detail::throw_on_error(rc, "ODBC: could not execute statement",
                           SQL_HANDLE_STMT, handle.get());
    return {.affected_rows = affected_row_count(handle.get())};
  }

  // prepared execution
  void execute_prepared(prepared_statement_t& prepared) {
    if constexpr (debug_enabled) {
      _handle.debug().log(log_category::statement,
                          "ODBC: executing prepared statement");
    }
    const auto rc = SQLExecute(prepared.native_handle());
    if (rc == SQL_NO_DATA) {
      return;
    }
    detail::throw_on_error(rc, "ODBC: could not execute prepared statement",
                           SQL_HANDLE_STMT, prepared.native_handle());
  }

  template <typename Statement>
  prepared_statement_t prepare_statement(const Statement& statement) {
    context_t context{this};
    const auto query = to_sql_string(context, statement);
    return prepared_statement_t{allocate_statement(), query, context._count,
                                _handle.config.get()};
  }

  template <typename PreparedStatement>
  prepared_statement_t& bind_prepared_parameters(PreparedStatement& statement) {
    auto& prepared =
        sqlpp::statement_handler_t{}.get_prepared_statement(statement);
    prepared._reset();
    sqlpp::statement_handler_t{}.bind_parameters(statement);
    return prepared;
  }

  template <typename PreparedCommand>
  command_result run_prepared_command(PreparedCommand& command) {
    auto& prepared = bind_prepared_parameters(command);
    execute_prepared(prepared);
    return {.affected_rows = affected_row_count(prepared.native_handle())};
  }

  //! select returns a result (which can be iterated row by row)
  template <typename Select>
  cursor_result_t _select(const Select& s) {
    context_t context{this};
    const auto query = to_sql_string(context, s);
    if constexpr (debug_enabled) {
      _handle.debug().log(log_category::statement, "ODBC: selecting: '{}'",
                          query);
    }
    auto statement = allocate_statement();
    const auto rc = SQLExecDirect(
        statement.get(),
        reinterpret_cast<SQLCHAR*>(const_cast<char*>(query.data())),
        static_cast<SQLINTEGER>(query.size()));
    if (rc != SQL_NO_DATA) {
      detail::throw_on_error(rc, "ODBC: could not execute select",
                             SQL_HANDLE_STMT, statement.get());
    }
    return {std::move(statement), _handle.config.get()};
  }

  template <typename Select>
  prepared_statement_t _prepare_select(const Select& s) {
    return prepare_statement(s);
  }

  template <typename PreparedSelect>
  cursor_result_t _run_prepared_select(PreparedSelect& s) {
    auto& prepared = bind_prepared_parameters(s);
    execute_prepared(prepared);
    return {prepared._statement, _handle.config.get()};
  }

  //! insert returns the number of affected rows
  //! Note: ODBC has no portable way to report the last inserted id. Use a
  //! select with a database-specific function if you need it.
  template <typename Insert>
  command_result _insert(const Insert& i) {
    context_t context{this};
    return execute_direct(to_sql_string(context, i));
  }

  template <typename Insert>
  prepared_statement_t _prepare_insert(const Insert& i) {
    return prepare_statement(i);
  }

  template <typename PreparedInsert>
  command_result _run_prepared_insert(PreparedInsert& i) {
    return run_prepared_command(i);
  }

  //! update returns the number of affected rows
  template <typename Update>
  command_result _update(const Update& u) {
    context_t context{this};
    return execute_direct(to_sql_string(context, u));
  }

  template <typename Update>
  prepared_statement_t _prepare_update(const Update& u) {
    return prepare_statement(u);
  }

  template <typename PreparedUpdate>
  command_result _run_prepared_update(PreparedUpdate& u) {
    return run_prepared_command(u);
  }

  //! delete_from returns the number of deleted rows
  template <typename Delete>
  command_result _delete_from(const Delete& r) {
    context_t context{this};
    return execute_direct(to_sql_string(context, r));
  }

  template <typename Delete>
  prepared_statement_t _prepare_delete_from(const Delete& r) {
    return prepare_statement(r);
  }

  template <typename PreparedDelete>
  command_result _run_prepared_delete_from(PreparedDelete& r) {
    return run_prepared_command(r);
  }

  //! Execute a single arbitrary statement (e.g. create a table)
  template <typename Execute>
  command_result _execute(const Execute& x) {
    context_t context{this};
    return execute_direct(to_sql_string(context, x));
  }

  template <typename Execute>
  prepared_statement_t _prepare_execute(const Execute& x) {
    return prepare_statement(x);
  }

  template <typename PreparedExecute>
  command_result _run_prepared_execute(PreparedExecute& x) {
    return run_prepared_command(x);
  }

  void set_autocommit(bool enabled) {
    detail::throw_on_error(
        SQLSetConnectAttr(
            native_handle(), SQL_ATTR_AUTOCOMMIT,
            reinterpret_cast<SQLPOINTER>(static_cast<SQLULEN>(
                enabled ? SQL_AUTOCOMMIT_ON : SQL_AUTOCOMMIT_OFF)),
            SQL_IS_UINTEGER),
        "ODBC: could not change autocommit mode", SQL_HANDLE_DBC,
        native_handle());
  }

  void end_transaction(SQLSMALLINT completion_type) {
    detail::throw_on_error(
        SQLEndTran(SQL_HANDLE_DBC, native_handle(), completion_type),
        completion_type == SQL_COMMIT ? "ODBC: could not commit transaction"
                                      : "ODBC: could not roll back transaction",
        SQL_HANDLE_DBC, native_handle());
    set_autocommit(true);
    _transaction_active = false;
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

  command_result operator()(std::string_view t) { return execute_direct(t); }

  template <typename T>
    requires(sqlpp::is_statement_v<T>)
  auto prepare(const T& t) {
    sqlpp::check_prepare_consistency(t).verify();
    sqlpp::check_compatibility<context_t>(t).verify();
    return sqlpp::statement_handler_t{}.prepare(t, *this);
  }

  //! set the transaction isolation level for this connection
  void set_default_isolation_level(isolation_level level) {
    SQLULEN mask{};
    switch (level) {
      case isolation_level::read_uncommitted:
        mask = SQL_TXN_READ_UNCOMMITTED;
        break;
      case isolation_level::read_committed:
        mask = SQL_TXN_READ_COMMITTED;
        break;
      case isolation_level::repeatable_read:
        mask = SQL_TXN_REPEATABLE_READ;
        break;
      case isolation_level::serializable:
        mask = SQL_TXN_SERIALIZABLE;
        break;
      case isolation_level::undefined:
        throw sqlpp::exception{"ODBC: invalid isolation level"};
    }
    detail::throw_on_error(
        SQLSetConnectAttr(native_handle(), SQL_ATTR_TXN_ISOLATION,
                          reinterpret_cast<SQLPOINTER>(mask), SQL_IS_UINTEGER),
        "ODBC: could not set the transaction isolation level", SQL_HANDLE_DBC,
        native_handle());
  }

  //! get the currently active transaction isolation level
  sqlpp::isolation_level get_default_isolation_level() {
    // The attribute is documented as a 32-bit mask, the wider buffer guards
    // against drivers that write SQLULEN.
    SQLULEN mask{0};
    detail::throw_on_error(
        SQLGetConnectAttr(native_handle(), SQL_ATTR_TXN_ISOLATION, &mask,
                          SQL_IS_UINTEGER, nullptr),
        "ODBC: could not get the transaction isolation level", SQL_HANDLE_DBC,
        native_handle());
    switch (mask) {
      case SQL_TXN_READ_UNCOMMITTED:
        return isolation_level::read_uncommitted;
      case SQL_TXN_READ_COMMITTED:
        return isolation_level::read_committed;
      case SQL_TXN_REPEATABLE_READ:
        return isolation_level::repeatable_read;
      case SQL_TXN_SERIALIZABLE:
        return isolation_level::serializable;
      default:
        return isolation_level::undefined;
    }
  }

  //! start a transaction by turning off autocommit; ODBC has no explicit
  //! BEGIN. If an isolation level is given, it is set on the connection and
  //! stays in effect for subsequent transactions, too.
  void start_transaction(isolation_level level = isolation_level::undefined) {
    if (level != isolation_level::undefined) {
      set_default_isolation_level(level);
    }
    set_autocommit(false);
    _transaction_active = true;
  }

  //! commit transaction
  void commit_transaction() { end_transaction(SQL_COMMIT); }

  //! rollback transaction
  void rollback_transaction() {
    if constexpr (debug_enabled) {
      _handle.debug().log(log_category::connection,
                          "ODBC: rolling back unfinished transaction");
    }
    end_transaction(SQL_ROLLBACK);
  }

  //! report a rollback failure (will be called by transactions in case of a
  //! rollback failure in the destructor)
  void report_rollback_failure(const std::string& message) noexcept {
    if constexpr (debug_enabled) {
      _handle.debug().log(log_category::connection, "rollback failure: {}",
                          message);
    }
  }

  //! check if transaction is active
  bool is_transaction_active() { return _transaction_active; }

  SQLHDBC native_handle() const { return _handle.native_handle(); }

  // Standard SQL string escaping (doubling single quotes). This works
  // without asking the driver, but be aware that some databases have
  // non-standard quirks (e.g. backslash escapes in MySQL's default mode).
  // Prefer prepared statement parameters over string literals.
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
}  // namespace sqlpp::odbc
