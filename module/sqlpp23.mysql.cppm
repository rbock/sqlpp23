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

#include <sqlpp23/mysql/mysql.h>

export module sqlpp23.mysql;

export namespace sqlpp::mysql {
using ::sqlpp::mysql::connection;
using ::sqlpp::mysql::connection_config;
using ::sqlpp::mysql::connection_pool;
using ::sqlpp::mysql::pooled_connection;
using ::sqlpp::mysql::context_t;

using ::sqlpp::mysql::command_result;
using ::sqlpp::mysql::exception;

using ::sqlpp::mysql::scoped_library_initializer_t;
using ::sqlpp::mysql::global_library_init;

using ::sqlpp::mysql::delete_from;
using ::sqlpp::mysql::update;

using ::sqlpp::mysql::assert_no_bool_cast;
using ::sqlpp::mysql::assert_no_full_outer_join_t;

using ::sqlpp::mysql::to_sql_string;
using ::sqlpp::mysql::quoted_name_to_sql_string;
using ::sqlpp::mysql::data_type_to_sql_string;
}
