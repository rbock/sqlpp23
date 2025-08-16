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

#include <sqlpp23/sqlite3/sqlite3.h>

export module sqlpp23.sqlite3;

export namespace sqlpp::sqlite3 {
using ::sqlpp::sqlite3::connection;
using ::sqlpp::sqlite3::connection_config;
using ::sqlpp::sqlite3::connection_pool;
using ::sqlpp::sqlite3::pooled_connection;
using ::sqlpp::sqlite3::context_t;

using ::sqlpp::sqlite3::command_result;
using ::sqlpp::sqlite3::exception;

using ::sqlpp::sqlite3::delete_from;
using ::sqlpp::sqlite3::update;
using ::sqlpp::sqlite3::insert_into;
using ::sqlpp::sqlite3::insert_or_replace;
using ::sqlpp::sqlite3::insert_or_ignore;

using ::sqlpp::sqlite3::assert_no_cast_to_date_time;
using ::sqlpp::sqlite3::assert_no_any_t;

using ::sqlpp::sqlite3::to_sql_string;
using ::sqlpp::sqlite3::nan_to_sql_string;
using ::sqlpp::sqlite3::inf_to_sql_string;
using ::sqlpp::sqlite3::neg_inf_to_sql_string;
}
