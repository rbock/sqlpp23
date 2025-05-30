/*
 * Copyright (c) 2024, Roland Bock
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

#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/tests/core/serialize_helpers.h>
#include <sqlpp23/tests/core/tables.h>

int main(int, char*[]) {
  // Keep existing test variables if they don't conflict
  const auto cond = sqlpp::value(true);
  const auto cond2 = sqlpp::value(false);
  const auto val = 11;
  const auto val2 = 13;
  const auto expr = sqlpp::value(17) + 4;

  // Case operands use parentheses where required.
  SQLPP_COMPARE(case_when(cond).then(val).else_(val),
                "CASE WHEN 1 THEN 11 ELSE 11 END");
  SQLPP_COMPARE(case_when(cond).then(val).else_(expr),
                "CASE WHEN 1 THEN 11 ELSE (17 + 4) END");
  SQLPP_COMPARE(case_when(cond).then(expr).else_(val),
                "CASE WHEN 1 THEN (17 + 4) ELSE 11 END");
  SQLPP_COMPARE(case_when(cond).then(expr).else_(expr),
                "CASE WHEN 1 THEN (17 + 4) ELSE (17 + 4) END");
  SQLPP_COMPARE(case_when(false or cond).then(val).else_(val),
                "CASE WHEN (0 OR 1) THEN 11 ELSE 11 END");
  SQLPP_COMPARE(case_when(false or cond).then(val).else_(expr),
                "CASE WHEN (0 OR 1) THEN 11 ELSE (17 + 4) END");
  SQLPP_COMPARE(case_when(false or cond).then(expr).else_(val),
                "CASE WHEN (0 OR 1) THEN (17 + 4) ELSE 11 END");
  SQLPP_COMPARE(case_when(false or cond).then(expr).else_(expr),
                "CASE WHEN (0 OR 1) THEN (17 + 4) ELSE (17 + 4) END");

  // Mulitple when/then pairs serialize as expected.
  SQLPP_COMPARE(case_when(cond).then(val).when(cond2).then(val2).else_(expr),
                "CASE WHEN 1 THEN 11 WHEN 0 THEN 13 ELSE (17 + 4) END");

  return 0;
}
