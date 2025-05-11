#pragma once

/*
 * Copyright (c) 2013 - 2015, Roland Bock
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

#include <sqlpp23/mysql/database/connection_config.h>
#include <sqlpp23/mysql/sqlpp_mysql.h>

namespace sqlpp::mysql::detail {
struct result_handle {
  MYSQL_RES* mysql_res;
  const connection_config* config;

  result_handle(MYSQL_RES* res, const connection_config* config_)
      : mysql_res{res}, config{config_} {}

  result_handle(const result_handle&) = delete;
  result_handle(result_handle&&) = default;
  result_handle& operator=(const result_handle&) = delete;
  result_handle& operator=(result_handle&&) = default;

  ~result_handle() {
    if (mysql_res)
      mysql_free_result(mysql_res);
  }

  bool operator!() const { return !mysql_res; }

  const debug_logger& debug() { return config->debug; }
};
}  // namespace sqlpp::mysql::detail
