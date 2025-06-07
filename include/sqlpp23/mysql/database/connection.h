#pragma once

/*
 * Copyright (c) 2013 - 2017, Roland Bock
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

#include <memory>
#include <string>

#include <sqlpp23/core/database/connection.h>
#include <sqlpp23/core/query/statement.h>
#include <sqlpp23/core/query/statement_handler.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/core/type_traits.h>
#include <sqlpp23/mysql/bind_result.h>
#include <sqlpp23/mysql/clause/delete_from.h>
#include <sqlpp23/mysql/clause/update.h>
#include <sqlpp23/mysql/constraints.h>
#include <sqlpp23/mysql/database/connection_config.h>
#include <sqlpp23/mysql/database/connection_handle.h>
#include <sqlpp23/mysql/database/exception.h>
#include <sqlpp23/mysql/database/serializer_context.h>
#include <sqlpp23/mysql/prepared_statement.h>
#include <sqlpp23/mysql/sqlpp_mysql.h>
#include <sqlpp23/mysql/text_result.h>
#include <sqlpp23/mysql/to_sql_string.h>

namespace sqlpp::mysql {
namespace detail {
struct mysql_thread_initializer {
  mysql_thread_initializer() {
    if (!mysql_thread_safe()) {
      throw sqlpp::exception{
          "MySQL error: Operating on a non-threadsafe client"};
    }
    mysql_thread_init();
  }

  ~mysql_thread_initializer() { mysql_thread_end(); }
};

inline void thread_init() {
  thread_local mysql_thread_initializer thread_initializer;
}

inline void execute_statement(connection_handle& handle,
                              std::string_view statement) {
  thread_init();

  if constexpr (debug_enabled) {
    handle.debug().log(log_category::statement, "Executing: '{}'", statement);
  }

  if (mysql_query(handle.native_handle(), statement.data())) {
    throw exception{mysql_error(handle.native_handle()),
                    mysql_errno(handle.native_handle())};
  }
}

inline void execute_prepared_statement(
    prepared_statement_t& prepared_statement) {
  thread_init();

  if constexpr (debug_enabled) {
    prepared_statement.debug().log(log_category::statement,
                                   "Executing prepared_statement");
  }

  if (mysql_stmt_bind_param(prepared_statement.native_handle().get(),
                            prepared_statement.parameters().data())) {
    throw exception{mysql_stmt_error(prepared_statement.native_handle().get()),
                    mysql_stmt_errno(prepared_statement.native_handle().get())};
  }

  if (mysql_stmt_execute(prepared_statement.native_handle().get())) {
    throw exception{mysql_stmt_error(prepared_statement.native_handle().get()),
                    mysql_stmt_errno(prepared_statement.native_handle().get())};
  }
}

}  // namespace detail

struct scoped_library_initializer_t {
  scoped_library_initializer_t(int argc = 0,
                               char** argv = nullptr,
                               char** groups = nullptr) {
    mysql_library_init(argc, argv, groups);
  }

  ~scoped_library_initializer_t() { mysql_library_end(); }
};

// This will also cleanup when the program shuts down
inline void global_library_init(int argc = 0,
                                char** argv = nullptr,
                                char** groups = nullptr) {
  static const auto global_init_and_end =
      scoped_library_initializer_t(argc, argv, groups);
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
  using _connection_base_t = connection_base;
  using _config_t = connection_config;
  using _config_ptr_t = std::shared_ptr<const _config_t>;
  using _handle_t = detail::connection_handle;

  using _prepared_statement_t = ::sqlpp::mysql::prepared_statement_t;

 private:
  friend sqlpp::statement_handler_t;

  bool _transaction_active{false};

  // direct execution

  command_result execute_impl(std::string_view statement) {
    execute_statement(_handle, statement);
    return {.affected_rows = mysql_affected_rows(_handle.native_handle())};
  }

  text_result_t select_impl(const std::string& statement) {
    execute_statement(_handle, statement);
    std::unique_ptr<MYSQL_RES, void (*)(MYSQL_RES*)> result = {
        mysql_store_result(_handle.native_handle()), mysql_free_result};

    if (!result) {
    throw exception{mysql_error(_handle.native_handle()),
                    mysql_errno(_handle.native_handle())};
    }

    return {std::move(result), _handle.config.get()};
  }

  insert_result insert_impl(const std::string& statement) {
    execute_statement(_handle, statement);

    return {.affected_rows = mysql_affected_rows(_handle.native_handle()),
            .last_insert_id = mysql_insert_id(_handle.native_handle())};
  }

  command_result update_impl(const std::string& statement) {
    execute_statement(_handle, statement);
    return {.affected_rows = mysql_affected_rows(_handle.native_handle())};
  }

  command_result delete_from_impl(const std::string& statement) {
    execute_statement(_handle, statement);
    return {.affected_rows = mysql_affected_rows(_handle.native_handle())};
  }

  // prepared execution
  prepared_statement_t prepare_impl(const std::string& statement,
                                    size_t no_of_parameters) {
    detail::thread_init();

    if constexpr (debug_enabled) {
      _handle.debug().log(log_category::statement, "Preparing: '{}'",
                          statement);
    }

    return prepared_statement_t(_handle.native_handle(), statement,
                                no_of_parameters, _handle.config.get());
  }

  bind_result_t run_prepared_select_impl(
      prepared_statement_t& prepared_statement,
      size_t no_of_columns) {
    detail::execute_prepared_statement(prepared_statement);
    return bind_result_t{prepared_statement.native_handle(), no_of_columns,
                         _handle.config.get()};
  }

  insert_result run_prepared_insert_impl(
      prepared_statement_t& prepared_statement) {
    detail::execute_prepared_statement(prepared_statement);
    return {.affected_rows = mysql_stmt_affected_rows(
                prepared_statement.native_handle().get()),
            .last_insert_id =
                mysql_stmt_insert_id(prepared_statement.native_handle().get())};
  }

  command_result run_prepared_update_impl(
      prepared_statement_t& prepared_statement) {
    detail::execute_prepared_statement(prepared_statement);
    return {.affected_rows = mysql_stmt_affected_rows(
                prepared_statement.native_handle().get())};
  }

  command_result run_prepared_delete_from_impl(
      prepared_statement_t& prepared_statement) {
    detail::execute_prepared_statement(prepared_statement);
    return {.affected_rows = mysql_stmt_affected_rows(
                prepared_statement.native_handle().get())};
  }

  //! execute
  template <typename Execute>
  command_result _execute(const Execute& i) {
    context_t context(this);
    const auto query = to_sql_string(context, i);
    return execute_impl(query);
  }

  template <typename Execute>
  _prepared_statement_t _prepare_execute(const Execute& u) {
    context_t context(this);
    const auto query = to_sql_string(context, u);
    return prepare_impl(query, parameters_of_t<std::decay_t<Execute>>::size());
  }

  template <typename PreparedExecute>
  command_result _run_prepared_execute(PreparedExecute& u) {
    sqlpp::statement_handler_t{}.bind_parameters(u);
    return run_prepared_update_impl(
        sqlpp::statement_handler_t{}.get_prepared_statement(u));
  }

  template <typename Select>
  text_result_t _select(const Select& s) {
    context_t context(this);
    const auto query = to_sql_string(context, s);
    return select_impl(query);
  }

  template <typename Select>
  _prepared_statement_t _prepare_select(const Select& s) {
    context_t context(this);
    const auto query = to_sql_string(context, s);
    return prepare_impl(query, parameters_of_t<std::decay_t<Select>>::size());
  }

  template <typename PreparedSelect>
  bind_result_t _run_prepared_select(PreparedSelect& s) {
    sqlpp::statement_handler_t{}.bind_parameters(s);
    return run_prepared_select_impl(
        sqlpp::statement_handler_t{}.get_prepared_statement(s),
        sqlpp::no_of_result_columns<std::decay_t<PreparedSelect>>::value);
  }

  //! insert returns the last auto_incremented id (or zero, if there is none)
  template <typename Insert>
  insert_result _insert(const Insert& i) {
    context_t context(this);
    const auto query = to_sql_string(context, i);
    return insert_impl(query);
  }

  template <typename Insert>
  _prepared_statement_t _prepare_insert(const Insert& i) {
    context_t context(this);
    const auto query = to_sql_string(context, i);
    return prepare_impl(query, parameters_of_t<std::decay_t<Insert>>::size());
  }

  template <typename PreparedInsert>
  insert_result _run_prepared_insert(PreparedInsert& i) {
    sqlpp::statement_handler_t{}.bind_parameters(i);
    return run_prepared_insert_impl(
        sqlpp::statement_handler_t{}.get_prepared_statement(i));
  }

  //! update returns the number of affected rows
  template <typename Update>
  command_result _update(const Update& u) {
    context_t context(this);
    const auto query = to_sql_string(context, u);
    return update_impl(query);
  }

  template <typename Update>
  _prepared_statement_t _prepare_update(const Update& u) {
    context_t context(this);
    const auto query = to_sql_string(context, u);
    return prepare_impl(query, parameters_of_t<std::decay_t<Update>>::size());
  }

  template <typename PreparedUpdate>
  command_result _run_prepared_update(PreparedUpdate& u) {
    sqlpp::statement_handler_t{}.bind_parameters(u);
    return run_prepared_update_impl(
        sqlpp::statement_handler_t{}.get_prepared_statement(u));
  }

  //! delete_from returns the number of deleted rows
  template <typename Delete>
  command_result _delete_from(const Delete& r) {
    context_t context(this);
    const auto query = to_sql_string(context, r);
    return delete_from_impl(query);
  }

  template <typename Delete>
  _prepared_statement_t _prepare_delete_from(const Delete& r) {
    context_t context(this);
    const auto query = to_sql_string(context, r);
    return prepare_impl(query, parameters_of_t<std::decay_t<Delete>>::size());
  }

  template <typename PreparedDelete>
  command_result _run_prepared_delete_from(PreparedDelete& r) {
    sqlpp::statement_handler_t{}.bind_parameters(r);
    return run_prepared_delete_from_impl(
        sqlpp::statement_handler_t{}.get_prepared_statement(r));
  }

 public:
  //! Direct execution
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

  //! Execute arbitrary statement (e.g. create a table).
  //! Essentially this calls mysql_query, see
  //! https://dev.mysql.com/doc/c-api/8.0/en/mysql-query.html Note:
  //!  * This usually only allows a single statement (unless configured
  //!  otherwise for the connection).
  //!  * If you are passing a statement with results, like a SELECT, you will
  //!  need to fetch results before issuing
  //!    the next statement on the same connection.
  command_result operator()(std::string_view t) { return execute_impl(t); }

  //! call prepare on the argument
  template <typename T>
    requires(sqlpp::is_statement_v<T>)
  auto prepare(const T& t) {
    sqlpp::check_prepare_consistency(t).verify();
    sqlpp::check_compatibility<context_t>(t).verify();
    return sqlpp::statement_handler_t{}.prepare(t, *this);
  }

  //! start transaction
  void start_transaction() {
    execute_statement(_handle, "START TRANSACTION");
    _transaction_active = true;
  }

  //! commit transaction
  void commit_transaction() {
    execute_statement(_handle, "COMMIT");
    _transaction_active = false;
  }

  //! rollback transaction
  void rollback_transaction() {
    if (debug_enabled) {
      _handle.debug().log(log_category::connection,
                          "Rolling back unfinished transaction");
    }
    execute_statement(_handle, "ROLLBACK");
    _transaction_active = false;
  }

  //! report a rollback failure (will be called by transactions in case of a
  //! rollback failure in the destructor)
  void report_rollback_failure(const std::string& message) noexcept {
    _handle.debug().log(log_category::connection, "Rollback error: {}",
                        message);
  }

  //! check if transaction is active
  bool is_transaction_active() { return _transaction_active; }

  MYSQL* native_handle() { return _handle.native_handle(); }

  std::string escape(const std::string_view& s) const {
    // Escape strings
    std::string result;
    result.resize((s.size() * 2) + 1);

    size_t length = mysql_real_escape_string(_handle.native_handle(),
                                             result.data(), s.data(), s.size());
    result.resize(length);
    return result;
  }

  const std::shared_ptr<const connection_config>& get_config() {
    return _handle.config;
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

}  // namespace sqlpp::mysql
