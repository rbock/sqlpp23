/*
 * Copyright (c) 2025, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
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

module;

#include <sqlpp23/postgresql/postgresql.h>

export module sqlpp23.postgresql;

export namespace sqlpp::postgresql {
using ::sqlpp::postgresql::connection;
using ::sqlpp::postgresql::connection_config;
using ::sqlpp::postgresql::connection_pool;
using ::sqlpp::postgresql::pooled_connection;
using ::sqlpp::postgresql::context_t;

using ::sqlpp::postgresql::command_result;

using ::sqlpp::postgresql::delete_from;
using ::sqlpp::postgresql::insert_into;
using ::sqlpp::postgresql::update;

using ::sqlpp::postgresql::connection_exception;
using ::sqlpp::postgresql::result_exception;

using ::sqlpp::postgresql::assert_no_cast_bool_to_numeric;
using ::sqlpp::postgresql::assert_no_unsigned;

using ::sqlpp::postgresql::to_sql_string;
using ::sqlpp::postgresql::data_type_to_sql_string;
}
