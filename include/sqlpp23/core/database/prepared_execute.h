#pragma once

/*
 * Copyright (c) 2013-2015, Roland Bock
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

#include <sqlpp23/core/database/parameter_list.h>
#include <sqlpp23/core/query/statement_handler.h>
#include <sqlpp23/core/result.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
template <typename Db, typename _Statement>
struct prepared_execute_t {
  using _parameter_list_t = make_parameter_list_t<_Statement>;
  using _prepared_statement_t = typename Db::_prepared_statement_t;

  prepared_execute_t() = default;
  explicit prepared_execute_t(_prepared_statement_t prepared_statement)
      : _prepared_statement(std::move(prepared_statement)) {}
  prepared_execute_t(const prepared_execute_t&) = default;
  prepared_execute_t(prepared_execute_t&&) = default;
  prepared_execute_t& operator=(const prepared_execute_t&) = default;
  prepared_execute_t& operator=(prepared_execute_t&&) = default;
  ~prepared_execute_t() = default;

  _parameter_list_t parameters = {};

 private:
  friend statement_handler_t;

  auto _run(Db& db) {
    return statement_handler_t{}.run_prepared_execute(*this, db);
  }

  void _bind_parameters() { parameters._bind(_prepared_statement); }

  _prepared_statement_t _prepared_statement;
};

template <typename Db, typename _Statement>
struct is_prepared_statement<prepared_execute_t<Db, _Statement>>
    : public std::true_type {};

}  // namespace sqlpp
