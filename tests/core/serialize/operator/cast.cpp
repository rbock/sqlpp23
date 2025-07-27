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

#include <sqlpp23/tests/core/all.h>

int main() {
  SQLPP_COMPARE(cast("7", as(sqlpp::boolean{})), "CAST('7' AS BOOLEAN)");
  SQLPP_COMPARE(cast("7", as(sqlpp::integral{})), "CAST('7' AS BIGINT)");
  SQLPP_COMPARE(cast("7", as(sqlpp::unsigned_integral{})), "CAST('7' AS BIGINT UNSIGNED)");
  SQLPP_COMPARE(cast("7", as(sqlpp::floating_point{})), "CAST('7' AS DOUBLE PRECISION)");
  SQLPP_COMPARE(cast("7", as(sqlpp::text{})), "CAST('7' AS VARCHAR)");
  SQLPP_COMPARE(cast("7", as(sqlpp::blob{})), "CAST('7' AS BLOB)");
  SQLPP_COMPARE(cast("7", as(sqlpp::date{})), "CAST('7' AS DATE)");
  SQLPP_COMPARE(cast("7", as(sqlpp::timestamp{})), "CAST('7' AS TIMESTAMP)");
  SQLPP_COMPARE(cast("7", as(sqlpp::time{})), "CAST('7' AS TIME)");

  return 0;
}
