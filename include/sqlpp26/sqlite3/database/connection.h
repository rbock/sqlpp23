#pragma once

/*
 * Copyright (c) 2013 - 2016, Roland Bock
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

#include <string>

#ifdef SQLPP_USE_SQLCIPHER
#include <sqlcipher/sqlite3.h>
#else
#include <sqlite3.h>
#endif
#include <sqlpp23/core/basic/schema.h>
#include <sqlpp23/core/database/connection.h>
#include <sqlpp23/core/database/transaction.h>
#include <sqlpp23/core/query/statement_handler.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/core/type_traits.h>
#include <sqlpp23/sqlite3/bind_result.h>
#include <sqlpp23/sqlite3/constraints.h>
#include <sqlpp23/sqlite3/database/connection_config.h>
#include <sqlpp23/sqlite3/database/connection_handle.h>
#include <sqlpp23/sqlite3/database/exception.h>
#include <sqlpp23/sqlite3/database/serializer_context.h>
#include <sqlpp23/sqlite3/prepared_statement.h>
#include <sqlpp23/sqlite3/to_sql_string.h>

namespace sqlpp::sqlite3 {

namespace detail {
inline prepared_statement_t prepare_statement(connection_handle& handle,
                                              std::string_view statement) {
  return prepared_statement_t{handle.native_handle(), statement,
                              handle.config.get()};
}
inline void execute_statement(connection_handle& handle,
                              prepared_statement_t& prepared) {
  auto rc = sqlite3_step(prepared.native_handle());
  switch (rc) {
    case SQLITE_OK:
    case SQLITE_ROW:  // might occur if execute is called with a select
    case SQLITE_DONE:
      return;
    default:
      if constexpr (debug_enabled) {
        handle.debug().log(log_category::statement,
                           "sqlite3_step return code: {}", rc);
      }
      throw exception{sqlite3_errmsg(handle.native_handle()), rc};
  }
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
    auto prepared = prepare_statement(_handle, statement);
    execute_statement(_handle, prepared);

    return {.affected_rows =
                static_cast<uint64_t>(sqlite3_changes(native_handle()))};
  }

  bind_result_t select_impl(const std::string& statement) {
    auto prepared = prepare_statement(_handle, statement);

    return {native_handle(), prepared._sqlite3_statement,
            _handle.config.get()};
  }

  insert_result insert_impl(const std::string& statement) {
    auto prepared = prepare_statement(_handle, statement);
    execute_statement(_handle, prepared);

    return {
        .affected_rows = static_cast<uint64_t>(sqlite3_changes(native_handle())),
        .last_insert_id =
            static_cast<uint64_t>(sqlite3_last_insert_rowid(native_handle()))};
  }

  command_result update_impl(const std::string& statement) {
    auto prepared = prepare_statement(_handle, statement);
    execute_statement(_handle, prepared);
    return {.affected_rows =
                static_cast<uint64_t>(sqlite3_changes(native_handle()))};
  }

  command_result delete_from_impl(const std::string& statement) {
    auto prepared = prepare_statement(_handle, statement);
    execute_statement(_handle, prepared);
    return {.affected_rows =
                static_cast<uint64_t>(sqlite3_changes(native_handle()))};
  }

  // prepared execution
  prepared_statement_t prepare_impl(const std::string& statement) {
    return prepare_statement(_handle, statement);
  }

  bind_result_t run_prepared_select_impl(
      prepared_statement_t& prepared_statement) {
    return {native_handle(), prepared_statement._sqlite3_statement,
            prepared_statement.config};
  }

  insert_result run_prepared_insert_impl(
      prepared_statement_t& prepared_statement) {
    execute_statement(_handle, prepared_statement);

    return {
        .affected_rows = static_cast<uint64_t>(sqlite3_changes(native_handle())),
        .last_insert_id =
            static_cast<uint64_t>(sqlite3_last_insert_rowid(native_handle()))};
  }

  command_result run_prepared_update_impl(
      prepared_statement_t& prepared_statement) {
    execute_statement(_handle, prepared_statement);

    return {.affected_rows =
                static_cast<uint64_t>(sqlite3_changes(native_handle()))};
  }

  command_result run_prepared_delete_from_impl(
      prepared_statement_t& prepared_statement) {
    execute_statement(_handle, prepared_statement);

    return {.affected_rows =
                static_cast<uint64_t>(sqlite3_changes(native_handle()))};
  }

  command_result run_prepared_execute_impl(
      prepared_statement_t& prepared_statement) {
    execute_statement(_handle, prepared_statement);

    return {.affected_rows =
                static_cast<uint64_t>(sqlite3_changes(native_handle()))};
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

  //! set the transaction isolation level for this connection
  void set_default_isolation_level(isolation_level level) {
    if (level == sqlpp::isolation_level::read_uncommitted) {
      execute_impl("pragma read_uncommitted = true");
    } else {
      execute_impl("pragma read_uncommitted = false");
    }
  }

  //! get the currently active transaction isolation level
  sqlpp::isolation_level get_default_isolation_level() {
    auto prepared = prepare_statement(_handle, "pragma read_uncommitted");
    execute_statement(_handle, prepared);

    int level = sqlite3_column_int(prepared._sqlite3_statement.get(), 0);

    return level == 0 ? sqlpp::isolation_level::serializable
                      : sqlpp::isolation_level::read_uncommitted;
  }

  //! start transaction
  void start_transaction() {
    auto prepared = prepare_statement(_handle, "BEGIN");
    execute_statement(_handle, prepared);
    _transaction_active = true;
  }

  //! commit transaction
  void commit_transaction() {
    auto prepared = prepare_statement(_handle, "COMMIT");
    execute_statement(_handle, prepared);
    _transaction_active = false;
  }

  //! rollback transaction with or without reporting the rollback
  void rollback_transaction() {
    if constexpr (debug_enabled) {
      _handle.debug().log(
          log_category::connection,
          "Sqlite3 warning: Rolling back unfinished transaction");
    }
    auto prepared = prepare_statement(_handle, "ROLLBACK");
    execute_statement(_handle, prepared);
    _transaction_active = false;
  }

  //! report a rollback failure (will be called by transactions in case of a
  //! rollback failure in the destructor)
  void report_rollback_failure(const std::string& message) noexcept {
    if constexpr (debug_enabled) {
      _handle.debug().log(log_category::connection,
                          "rollback failure: ", message);
    }
  }

  //! check if transaction is active
  bool is_transaction_active() { return _transaction_active; }

  //! get the last inserted id
  uint64_t last_insert_id() noexcept {
    return static_cast<uint64_t>(sqlite3_last_insert_rowid(native_handle()));
  }

  ::sqlite3* native_handle() const { return _handle.native_handle(); }

  schema_t attach(const connection_config& config, const std::string& name) {
    context_t context{this};
    auto prepared = prepare_statement(
        _handle, "ATTACH " +
                     sqlpp::to_sql_string(context, config.path_to_database) +
                     " AS " + sqlpp::quoted_name_to_sql_string(context, name));
    execute_statement(_handle, prepared);

    return {name};
  }

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
}  // namespace sqlpp::sqlite3
