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

int main(int, char*[]) {
  {
    SQLPP_COMPARE(sqlpp::coalesce("a"), "COALESCE('a')");
    SQLPP_COMPARE(sqlpp::coalesce("a", "b"), "COALESCE('a', 'b')");
    SQLPP_COMPARE(sqlpp::coalesce("a", sqlpp::dynamic(true, "b"), "c"),
                  "COALESCE('a', 'b', 'c')");
    SQLPP_COMPARE(sqlpp::coalesce("a", sqlpp::dynamic(false, "b"), "c"),
                  "COALESCE('a', NULL, 'c')");
  }
  {
    SQLPP_COMPARE(sqlpp::concat("a"), "CONCAT('a')");
    SQLPP_COMPARE(sqlpp::concat("a", "b"), "CONCAT('a', 'b')");
    SQLPP_COMPARE(sqlpp::concat("a", sqlpp::dynamic(true, "b"), "c"),
                  "CONCAT('a', 'b', 'c')");
    SQLPP_COMPARE(sqlpp::concat("a", sqlpp::dynamic(false, "b"), "c"),
                  "CONCAT('a', NULL, 'c')");
  }
  {
    SQLPP_COMPARE(sqlpp::current_date, "CURRENT_DATE");
  }
  {
    SQLPP_COMPARE(sqlpp::current_time, "CURRENT_TIME");
  }
  {
    SQLPP_COMPARE(sqlpp::current_timestamp, "CURRENT_TIMESTAMP");
  }
  {
    auto ctx = sqlpp::mock_db::context_t{};

    SQLPP_COMPARE(flatten(ctx, test::TabFoo{}.id), "tab_foo.id");
    SQLPP_COMPARE(flatten(ctx, from(test::TabFoo{})), " FROM tab_foo");
    SQLPP_COMPARE(flatten(ctx, test::TabFoo{}.id).asc(), "tab_foo.id ASC");
  }
  {
    // Note: The tests below serialize the string returned by `get_sql_name`.
    //       That serialization adds ticks.
    SQLPP_COMPARE(get_sql_name(test::TabFoo{}), "'tab_foo'");
    SQLPP_COMPARE(get_sql_name(test::TabFoo{}.id), "'id'");
  }
  {
    const auto bar = test::TabBar{};

    // Single column.
    SQLPP_COMPARE(lower(bar.textN), "LOWER(tab_bar.text_n)");

    // Expression.
    SQLPP_COMPARE(lower(bar.textN + "suffix"),
                  "LOWER(CONCAT(tab_bar.text_n, 'suffix'))");

    // With sub select.
    SQLPP_COMPARE(lower(select(sqlpp::value("something").as(sqlpp::alias::a))),
                  "LOWER(SELECT 'something' AS a)");
  }
  {
    const auto bar = test::TabBar{};

    // Single column.
    SQLPP_COMPARE(trim(bar.textN), "TRIM(tab_bar.text_n)");

    // Expression.
    SQLPP_COMPARE(trim(bar.textN + "suffix"),
                  "TRIM(CONCAT(tab_bar.text_n, 'suffix'))");

    // With sub select.
    SQLPP_COMPARE(trim(select(sqlpp::value("something").as(sqlpp::alias::a))),
                  "TRIM(SELECT 'something' AS a)");
  }
  {
    const auto bar = test::TabBar{};

    // Single column.
    SQLPP_COMPARE(upper(bar.textN), "UPPER(tab_bar.text_n)");

    // Expression.
    SQLPP_COMPARE(upper(bar.textN + "suffix"),
                  "UPPER(CONCAT(tab_bar.text_n, 'suffix'))");

    // With sub select.
    SQLPP_COMPARE(upper(select(sqlpp::value("something").as(sqlpp::alias::a))),
                  "UPPER(SELECT 'something' AS a)");
  }
  return 0;
}
