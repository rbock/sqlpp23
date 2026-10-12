/*
 * Copyright (c) 2026
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
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

#include <sqlpp23/duck_db/duck_db.h>

export module sqlpp23.duck_db;

export namespace sqlpp::duck_db {
using ::sqlpp::duck_db::bind_parameter;
using ::sqlpp::duck_db::read_field;
using ::sqlpp::duck_db::connection;
using ::sqlpp::duck_db::connection_config;
using ::sqlpp::duck_db::connection_pool;
using ::sqlpp::duck_db::pooled_connection;
using ::sqlpp::duck_db::context_t;

using ::sqlpp::duck_db::command_result;
using ::sqlpp::duck_db::exception;

using ::sqlpp::duck_db::delete_from;
using ::sqlpp::duck_db::update;
using ::sqlpp::duck_db::insert_into;
using ::sqlpp::duck_db::insert_or_replace;
using ::sqlpp::duck_db::insert_or_ignore;

using ::sqlpp::duck_db::assert_no_using_t;

using ::sqlpp::duck_db::to_sql_string;
using ::sqlpp::duck_db::nan_to_sql_string;
using ::sqlpp::duck_db::inf_to_sql_string;
using ::sqlpp::duck_db::neg_inf_to_sql_string;
using ::sqlpp::duck_db::blob_to_sql_string;
}
