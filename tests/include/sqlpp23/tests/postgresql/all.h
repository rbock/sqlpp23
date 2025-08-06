/*
 * Copyright (c) 2025, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
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

// Note: To work around a GCC bug, make sure that for any standard header that
// is both included and imported from a module, the #include directive comes
// before the import declaration. For details see
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=114795#c3

#include <cassert>
#include <chrono>
#include <memory>
#include <print>

#include <sqlpp23/core/name/create_name_tag.h>
#include <sqlpp23/tests/core/assert_throw.h>
#include <sqlpp23/tests/core/result_helpers.h>
#include <sqlpp23/tests/postgresql/make_test_connection.h>
#include <sqlpp23/tests/postgresql/serialize_helpers.h>

#ifdef BUILD_WITH_MODULES
import sqlpp23.core;
import sqlpp23.postgresql;
import sqlpp23.test.postgresql.tables;
#else
#include <sqlpp23/core/database/connection_pool.h>
#include <sqlpp23/postgresql/postgresql.h>
#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/tests/postgresql/tables.h>
#endif
