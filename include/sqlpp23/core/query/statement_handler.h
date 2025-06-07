#pragma once

/*
 * Copyright (c) 2025, Roland Bock
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

#include <utility>

// Used as friend class to reduce the exposed API in statement and connection.
namespace sqlpp {
class statement_handler_t {
 public:
  template <typename Statement, typename Db>
  auto run(Statement&& statement, Db& db) {
    return std::forward<Statement>(statement)._run(db);
  }

  template <typename Statement, typename Db>
  auto prepare(Statement&& statement, Db& db) {
    return std::forward<Statement>(statement)._prepare(db);
  }

  template <typename Statement, typename Db>
  auto execute(Statement&& statement, Db& db) {
    return db._execute(std::forward<Statement>(statement));
  }

  template <typename Statement, typename Db>
  auto delete_from(Statement&& statement, Db& db) {
    return db._delete_from(std::forward<Statement>(statement));
  }

  template <typename Statement, typename Db>
  auto insert(Statement&& statement, Db& db) {
    return db._insert(std::forward<Statement>(statement));
  }

  template <typename Statement, typename Db>
  auto select(Statement&& statement, Db& db) {
    return db._select(std::forward<Statement>(statement));
  }

  template <typename Statement, typename Db>
  auto update(Statement&& statement, Db& db) {
    return db._update(std::forward<Statement>(statement));
  }

  template <typename Statement, typename Db>
  auto prepare_execute(Statement&& statement, Db& db) {
    return db._prepare_execute(std::forward<Statement>(statement));
  }

  template <typename Statement, typename Db>
  auto prepare_delete_from(Statement&& statement, Db& db) {
    return db._prepare_delete_from(std::forward<Statement>(statement));
  }

  template <typename Statement, typename Db>
  auto prepare_insert(Statement&& statement, Db& db) {
    return db._prepare_insert(std::forward<Statement>(statement));
  }

  template <typename Statement, typename Db>
  auto prepare_select(Statement&& statement, Db& db) {
    return db._prepare_select(std::forward<Statement>(statement));
  }

  template <typename Statement, typename Db>
  auto prepare_update(Statement&& statement, Db& db) {
    return db._prepare_update(std::forward<Statement>(statement));
  }

  template <typename Statement, typename Db>
  auto run_prepared_execute(Statement&& statement, Db& db) {
    return db._run_prepared_execute(std::forward<Statement>(statement));
  }

  template <typename Statement, typename Db>
  auto run_prepared_delete_from(Statement&& statement, Db& db) {
    return db._run_prepared_delete_from(std::forward<Statement>(statement));
  }

  template <typename Statement, typename Db>
  auto run_prepared_insert(Statement&& statement, Db& db) {
    return db._run_prepared_insert(std::forward<Statement>(statement));
  }

  template <typename Statement, typename Db>
  auto run_prepared_select(Statement&& statement, Db& db) {
    return db._run_prepared_select(std::forward<Statement>(statement));
  }

  template <typename Statement, typename Db>
  auto run_prepared_update(Statement&& statement, Db& db) {
    return db._run_prepared_update(std::forward<Statement>(statement));
  }

  template <typename PreparedStatement>
  auto bind_parameters(PreparedStatement& statement) {
    return statement._bind_parameters();
  }

  template <typename PreparedStatement>
  auto& get_prepared_statement(PreparedStatement& statement) {
    return statement._prepared_statement;
  }
};

}  // namespace sqlpp
