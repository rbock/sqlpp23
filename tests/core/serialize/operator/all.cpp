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

SQLPP_CREATE_NAME_TAG(v);

int main(int, char*[]) {
  {
    const auto val = sqlpp::value(17);

    SQLPP_COMPARE(any(select(val.as(v))), "ANY (SELECT 17 AS v)");
    SQLPP_COMPARE(val == any(select(val.as(v))), "17 = ANY (SELECT 17 AS v)");
  }

  {
    const auto val = sqlpp::value(1);
    const auto expr = sqlpp::value(17) + 4;

    // Operands are enclosed in parenheses where required.
    SQLPP_COMPARE(val + val, "1 + 1");
    SQLPP_COMPARE(val - val, "1 - 1");
    SQLPP_COMPARE(val * val, "1 * 1");
    SQLPP_COMPARE(val / val, "1 / 1");
    SQLPP_COMPARE(val % val, "1 % 1");

    SQLPP_COMPARE(val + expr, "1 + (17 + 4)");
    SQLPP_COMPARE(val - expr, "1 - (17 + 4)");
    SQLPP_COMPARE(val * expr, "1 * (17 + 4)");
    SQLPP_COMPARE(val / expr, "1 / (17 + 4)");
    SQLPP_COMPARE(val % expr, "1 % (17 + 4)");

    SQLPP_COMPARE(expr + val, "(17 + 4) + 1");
    SQLPP_COMPARE(expr - val, "(17 + 4) - 1");
    SQLPP_COMPARE(expr * val, "(17 + 4) * 1");
    SQLPP_COMPARE(expr / val, "(17 + 4) / 1");
    SQLPP_COMPARE(expr % val, "(17 + 4) % 1");

    SQLPP_COMPARE(expr + expr, "(17 + 4) + (17 + 4)");
    SQLPP_COMPARE(expr - expr, "(17 + 4) - (17 + 4)");
    SQLPP_COMPARE(expr * expr, "(17 + 4) * (17 + 4)");
    SQLPP_COMPARE(expr / expr, "(17 + 4) / (17 + 4)");
    SQLPP_COMPARE(expr % expr, "(17 + 4) % (17 + 4)");

    // Same for unary expressions.
    SQLPP_COMPARE(-val, "-1");
    SQLPP_COMPARE(-val + val, "(-1) + 1");
    SQLPP_COMPARE(-expr, "-(17 + 4)");

    const auto text = sqlpp::value("a");
    const auto text_expr = sqlpp::value("b") + "c";

    // Same for concatenation.
    SQLPP_COMPARE(text + text, "CONCAT('a', 'a')");
    SQLPP_COMPARE(text + text_expr, "CONCAT('a', CONCAT('b', 'c'))");
    SQLPP_COMPARE(text_expr + text, "CONCAT(CONCAT('b', 'c'), 'a')");
    SQLPP_COMPARE(text_expr + text_expr,
                  "CONCAT(CONCAT('b', 'c'), CONCAT('b', 'c'))");

    // Arithmetic expressions can be named with AS
    SQLPP_COMPARE((val + val).as(sqlpp::alias::a), "(1 + 1) AS a");
    SQLPP_COMPARE((val - val).as(sqlpp::alias::a), "(1 - 1) AS a");
    SQLPP_COMPARE((val * val).as(sqlpp::alias::a), "(1 * 1) AS a");
    SQLPP_COMPARE((val / val).as(sqlpp::alias::a), "(1 / 1) AS a");
    SQLPP_COMPARE((val % val).as(sqlpp::alias::a), "(1 % 1) AS a");

    // Arithmetic expressions can be compared
    SQLPP_COMPARE((val + val) < 17, "(1 + 1) < 17");
    SQLPP_COMPARE((val - val) < 17, "(1 - 1) < 17");
    SQLPP_COMPARE((val * val) < 17, "(1 * 1) < 17");
    SQLPP_COMPARE((val / val) < 17, "(1 / 1) < 17");
    SQLPP_COMPARE((val % val) < 17, "(1 % 1) < 17");
    SQLPP_COMPARE(-val < 17, "(-1) < 17");
    SQLPP_COMPARE((text + text) < "z", "CONCAT('a', 'a') < 'z'");
  }
  {
    const auto val = sqlpp::value(17);
    const auto expr = sqlpp::value(17) + 4;

    const auto col_id = test::TabFoo{}.id;

    SQLPP_COMPARE(val.as(v), "17 AS v");
    SQLPP_COMPARE(expr.as(v), "(17 + 4) AS v");
    SQLPP_COMPARE(count(val).as(v), "COUNT(17) AS v");

    SQLPP_COMPARE(select_columns(dynamic(false, val.as(v))), "NULL AS v");
    SQLPP_COMPARE(select_columns(dynamic(false, expr.as(v))), "NULL AS v");
    SQLPP_COMPARE(select_columns(dynamic(false, count(val).as(v))),
                  "NULL AS v");
    SQLPP_COMPARE(select_columns(dynamic(false, col_id.as(v))), "NULL AS v");
  }
  {
    constexpr auto t = test::TabFoo{};
    const auto val = sqlpp::value(17);

    // Operands in assignments are enclosed in parentheses as required.
    SQLPP_COMPARE(t.intN = val, "int_n = 17");
    SQLPP_COMPARE(t.intN = val + 4, "int_n = (17 + 4)");
    SQLPP_COMPARE(t.intN = std::nullopt, "int_n = NULL");
  }
  {
    const auto val = sqlpp::value(1);
    const auto expr = sqlpp::value(17) + 4;

    // Operands are enclosed in parenheses where required
    SQLPP_COMPARE(val.between(val, val), "1 BETWEEN 1 AND 1");
    SQLPP_COMPARE(val.between(val, expr), "1 BETWEEN 1 AND (17 + 4)");
    SQLPP_COMPARE(val.between(expr, val), "1 BETWEEN (17 + 4) AND 1");
    SQLPP_COMPARE(val.between(expr, expr), "1 BETWEEN (17 + 4) AND (17 + 4)");
    SQLPP_COMPARE(expr.between(val, val), "(17 + 4) BETWEEN 1 AND 1");
    SQLPP_COMPARE(expr.between(val, expr), "(17 + 4) BETWEEN 1 AND (17 + 4)");
    SQLPP_COMPARE(expr.between(expr, val), "(17 + 4) BETWEEN (17 + 4) AND 1");
    SQLPP_COMPARE(expr.between(expr, expr),
                  "(17 + 4) BETWEEN (17 + 4) AND (17 + 4)");

    SQLPP_COMPARE(val.between(val, val) and true, "(1 BETWEEN 1 AND 1) AND 1");
  }
  {
    const auto val = sqlpp::value(1);
    const auto expr = sqlpp::value(17) + 4;

    // Operands are enclosed in parentheses where required.
    SQLPP_COMPARE(val & val, "1 & 1");
    SQLPP_COMPARE(val | val, "1 | 1");
    SQLPP_COMPARE(val ^ val, "1 ^ 1");
    SQLPP_COMPARE(val << val, "1 << 1");
    SQLPP_COMPARE(val >> val, "1 >> 1");

    SQLPP_COMPARE(val & expr, "1 & (17 + 4)");
    SQLPP_COMPARE(val | expr, "1 | (17 + 4)");
    SQLPP_COMPARE(val ^ expr, "1 ^ (17 + 4)");
    SQLPP_COMPARE(val << expr, "1 << (17 + 4)");
    SQLPP_COMPARE(val >> expr, "1 >> (17 + 4)");

    SQLPP_COMPARE(expr & val, "(17 + 4) & 1");
    SQLPP_COMPARE(expr | val, "(17 + 4) | 1");
    SQLPP_COMPARE(expr ^ val, "(17 + 4) ^ 1");
    SQLPP_COMPARE(expr << val, "(17 + 4) << 1");
    SQLPP_COMPARE(expr >> val, "(17 + 4) >> 1");

    SQLPP_COMPARE(expr & expr, "(17 + 4) & (17 + 4)");
    SQLPP_COMPARE(expr | expr, "(17 + 4) | (17 + 4)");
    SQLPP_COMPARE(expr ^ expr, "(17 + 4) ^ (17 + 4)");
    SQLPP_COMPARE(expr << expr, "(17 + 4) << (17 + 4)");
    SQLPP_COMPARE(expr >> expr, "(17 + 4) >> (17 + 4)");

    // Same for unary operators
    SQLPP_COMPARE(~val, "~1");
    SQLPP_COMPARE(~expr, "~(17 + 4)");
  }
  {
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
  }
  {
    SQLPP_COMPARE(cast("7", as(sqlpp::boolean{})), "CAST('7' AS BOOLEAN)");
    SQLPP_COMPARE(cast("7", as(sqlpp::integral{})), "CAST('7' AS BIGINT)");
    SQLPP_COMPARE(cast("7", as(sqlpp::unsigned_integral{})),
                  "CAST('7' AS BIGINT UNSIGNED)");
    SQLPP_COMPARE(cast("7", as(sqlpp::floating_point{})),
                  "CAST('7' AS DOUBLE PRECISION)");
    SQLPP_COMPARE(cast("7", as(sqlpp::text{})), "CAST('7' AS VARCHAR)");
    SQLPP_COMPARE(cast("7", as(sqlpp::blob{})), "CAST('7' AS BLOB)");
    SQLPP_COMPARE(cast("7", as(sqlpp::date{})), "CAST('7' AS DATE)");
    SQLPP_COMPARE(cast("7", as(sqlpp::timestamp{})), "CAST('7' AS TIMESTAMP)");
    SQLPP_COMPARE(cast("7", as(sqlpp::time{})), "CAST('7' AS TIME)");
  }
  {
    const auto val = sqlpp::value(1);
    const auto expr = sqlpp::value(17) + 4;

    // Operands are enclosed in parentheses where required.
    SQLPP_COMPARE(val < val, "1 < 1");
    SQLPP_COMPARE(val <= val, "1 <= 1");
    SQLPP_COMPARE(val == val, "1 = 1");
    SQLPP_COMPARE(val != val, "1 <> 1");
    SQLPP_COMPARE(val >= val, "1 >= 1");
    SQLPP_COMPARE(val > val, "1 > 1");
    SQLPP_COMPARE(val.is_distinct_from(val), "1 IS DISTINCT FROM 1");
    SQLPP_COMPARE(val.is_not_distinct_from(val), "1 IS NOT DISTINCT FROM 1");

    SQLPP_COMPARE(val < expr, "1 < (17 + 4)");
    SQLPP_COMPARE(val <= expr, "1 <= (17 + 4)");
    SQLPP_COMPARE(val == expr, "1 = (17 + 4)");
    SQLPP_COMPARE(val != expr, "1 <> (17 + 4)");
    SQLPP_COMPARE(val >= expr, "1 >= (17 + 4)");
    SQLPP_COMPARE(val > expr, "1 > (17 + 4)");
    SQLPP_COMPARE(val.is_distinct_from(expr), "1 IS DISTINCT FROM (17 + 4)");
    SQLPP_COMPARE(val.is_not_distinct_from(expr),
                  "1 IS NOT DISTINCT FROM (17 + 4)");

    SQLPP_COMPARE(expr < val, "(17 + 4) < 1");
    SQLPP_COMPARE(expr <= val, "(17 + 4) <= 1");
    SQLPP_COMPARE(expr == val, "(17 + 4) = 1");
    SQLPP_COMPARE(expr != val, "(17 + 4) <> 1");
    SQLPP_COMPARE(expr >= val, "(17 + 4) >= 1");
    SQLPP_COMPARE(expr > val, "(17 + 4) > 1");
    SQLPP_COMPARE(expr.is_distinct_from(val), "(17 + 4) IS DISTINCT FROM 1");
    SQLPP_COMPARE(expr.is_not_distinct_from(val),
                  "(17 + 4) IS NOT DISTINCT FROM 1");

    SQLPP_COMPARE(expr < expr, "(17 + 4) < (17 + 4)");
    SQLPP_COMPARE(expr <= expr, "(17 + 4) <= (17 + 4)");
    SQLPP_COMPARE(expr == expr, "(17 + 4) = (17 + 4)");
    SQLPP_COMPARE(expr != expr, "(17 + 4) <> (17 + 4)");
    SQLPP_COMPARE(expr >= expr, "(17 + 4) >= (17 + 4)");
    SQLPP_COMPARE(expr > expr, "(17 + 4) > (17 + 4)");
    SQLPP_COMPARE(expr.is_distinct_from(expr),
                  "(17 + 4) IS DISTINCT FROM (17 + 4)");
    SQLPP_COMPARE(expr.is_not_distinct_from(expr),
                  "(17 + 4) IS NOT DISTINCT FROM (17 + 4)");

    // Same for unary operators
    SQLPP_COMPARE(val.is_null(), "1 IS NULL");
    SQLPP_COMPARE(val.is_not_null(), "1 IS NOT NULL");

    SQLPP_COMPARE(expr.is_null(), "(17 + 4) IS NULL");
    SQLPP_COMPARE(expr.is_not_null(), "(17 + 4) IS NOT NULL");
  }
  {
    const auto val = sqlpp::value(17);

    SQLPP_COMPARE(exists(select(val.as(v))), "EXISTS (SELECT 17 AS v)");
    SQLPP_COMPARE(true and exists(select(val.as(v))),
                  "1 AND EXISTS (SELECT 17 AS v)");
    SQLPP_COMPARE(exists(select(val.as(v))) and true,
                  "EXISTS (SELECT 17 AS v) AND 1");

    SQLPP_COMPARE(exists(select(val.as(v))).as(sqlpp::alias::exists_),
                  "EXISTS (SELECT 17 AS v) AS exists_");
  }
  {
    const auto val = sqlpp::value(17);
    const auto expr = sqlpp::value(17) + 4;
    using expr_t = typename std::decay<decltype(expr)>::type;

    // IN expression with single select or other singe expression: No extra
    // parentheses.
    SQLPP_COMPARE(val.in(val), "17 IN (17)");
    SQLPP_COMPARE(val.in(expr), "17 IN (17 + 4)");
    SQLPP_COMPARE(val.in(select(val.as(v))), "17 IN (SELECT 17 AS v)");

    SQLPP_COMPARE(val.not_in(val), "17 NOT IN (17)");
    SQLPP_COMPARE(val.not_in(expr), "17 NOT IN (17 + 4)");
    SQLPP_COMPARE(val.not_in(select(val.as(v))), "17 NOT IN (SELECT 17 AS v)");

    // IN expressions with multiple arguments require inner parentheses.
    SQLPP_COMPARE(val.in(1, select(val.as(v)), 23),
                  "17 IN (1, (SELECT 17 AS v), 23)");
    SQLPP_COMPARE(val.in(std::vector<int>{17, 18, 19}), "17 IN (17, 18, 19)");
    SQLPP_COMPARE(val.in(std::vector<expr_t>{expr, expr, expr}),
                  "17 IN ((17 + 4), (17 + 4), (17 + 4))");

    SQLPP_COMPARE(val.not_in(1, select(val.as(v))),
                  "17 NOT IN (1, (SELECT 17 AS v))");
    SQLPP_COMPARE(val.not_in(std::vector<int>{17, 18, 19}),
                  "17 NOT IN (17, 18, 19)");
    SQLPP_COMPARE(val.not_in(std::vector<expr_t>{expr, expr, expr}),
                  "17 NOT IN ((17 + 4), (17 + 4), (17 + 4))");

    // IN expressions with no arguments are an error in SQL. No magic
    // protection.
    SQLPP_COMPARE(val.in(std::vector<expr_t>{}), "17 IN ()");
    SQLPP_COMPARE(val.not_in(std::vector<expr_t>{}), "17 NOT IN ()");

  }
  {
    const auto val = sqlpp::value(true);
    const auto expr = sqlpp::value(17) > 15;

    // Operands are enclosed in parenheses where required
    SQLPP_COMPARE(val and val, "1 AND 1");
    SQLPP_COMPARE(val and expr, "1 AND (17 > 15)");
    SQLPP_COMPARE(expr and val, "(17 > 15) AND 1");
    SQLPP_COMPARE(expr and expr, "(17 > 15) AND (17 > 15)");

    SQLPP_COMPARE(val or val, "1 OR 1");
    SQLPP_COMPARE(val or expr, "1 OR (17 > 15)");
    SQLPP_COMPARE(expr or val, "(17 > 15) OR 1");
    SQLPP_COMPARE(expr or expr, "(17 > 15) OR (17 > 15)");

    SQLPP_COMPARE(not val, "NOT 1");
    SQLPP_COMPARE(not expr, "NOT (17 > 15)");

    // Combined logical expression.
    SQLPP_COMPARE(not val and not expr, "(NOT 1) AND (NOT (17 > 15))");
    SQLPP_COMPARE(not val or not expr, "(NOT 1) OR (NOT (17 > 15))");
    SQLPP_COMPARE(not(val and expr), "NOT (1 AND (17 > 15))");
    SQLPP_COMPARE(not(val or expr), "NOT (1 OR (17 > 15))");

    // Chains are not nested in parentheses.
    SQLPP_COMPARE(val and val and val and val and val,
                  "1 AND 1 AND 1 AND 1 AND 1");
    SQLPP_COMPARE(val or val or val or val or val, "1 OR 1 OR 1 OR 1 OR 1");

    // Broken chains use parentheses for the respective blocks.
    SQLPP_COMPARE((val and val and val) or (val and val),
                  "(1 AND 1 AND 1) OR (1 AND 1)");
    SQLPP_COMPARE((val or val or val) and (val or val),
                  "(1 OR 1 OR 1) AND (1 OR 1)");

    // NOT is not chained gracefully, but hey, don't do that anyways.
    SQLPP_COMPARE(not not not val, "NOT (NOT (NOT 1))");

    // Operands are enclosed in parentheses where required or completely dropped
    // if inactive
    SQLPP_COMPARE(val and dynamic(true, val), "1 AND 1");
    SQLPP_COMPARE(val and dynamic(true, expr), "1 AND (17 > 15)");
    SQLPP_COMPARE(expr and dynamic(true, val), "(17 > 15) AND 1");
    SQLPP_COMPARE(expr and dynamic(true, expr), "(17 > 15) AND (17 > 15)");

    SQLPP_COMPARE(val or dynamic(true, val), "1 OR 1");
    SQLPP_COMPARE(val or dynamic(true, expr), "1 OR (17 > 15)");
    SQLPP_COMPARE(expr or dynamic(true, val), "(17 > 15) OR 1");
    SQLPP_COMPARE(expr or dynamic(true, expr), "(17 > 15) OR (17 > 15)");

    SQLPP_COMPARE(val and dynamic(false, val), "1");
    SQLPP_COMPARE(val and dynamic(false, expr), "1");
    SQLPP_COMPARE(expr and dynamic(false, val), "17 > 15");
    SQLPP_COMPARE(expr and dynamic(false, expr), "17 > 15");

    SQLPP_COMPARE(val or dynamic(false, val), "1");
    SQLPP_COMPARE(val or dynamic(false, expr), "1");
    SQLPP_COMPARE(expr or dynamic(false, val), "17 > 15");
    SQLPP_COMPARE(expr or dynamic(false, expr), "17 > 15");

    // Chained partially dynamic expressions
    SQLPP_COMPARE(val and dynamic(true, val) and expr, "1 AND 1 AND (17 > 15)");
    SQLPP_COMPARE(val and dynamic(false, val) and expr, "1 AND (17 > 15)");

    SQLPP_COMPARE(val or dynamic(true, val) or expr, "1 OR 1 OR (17 > 15)");
    SQLPP_COMPARE(val or dynamic(false, val) or expr, "1 OR (17 > 15)");

    // More complex expressions
    SQLPP_COMPARE((val and dynamic(true, expr)) or dynamic(true, val),
                  "(1 AND (17 > 15)) OR 1");
    // The extra parentheses are not great, but also difficult to avoid and not
    // a problem I believe.
    SQLPP_COMPARE((val and dynamic(false, expr)) or dynamic(true, val),
                  "(1) OR 1");
    SQLPP_COMPARE((val and dynamic(true, expr)) or dynamic(false, val),
                  "1 AND (17 > 15)");
    SQLPP_COMPARE((val and dynamic(false, expr)) or dynamic(false, val), "1")
  }
  {
    const auto val = sqlpp::value(1);
    const auto expr = sqlpp::value(17) + 4;

    // Operands are enclosed in parentheses where required.
    SQLPP_COMPARE(val.asc(), "1 ASC");
    SQLPP_COMPARE(val.desc(), "1 DESC");
    SQLPP_COMPARE(val.order(sqlpp::sort_type::asc), "1 ASC");
    SQLPP_COMPARE(val.order(sqlpp::sort_type::desc), "1 DESC");

    SQLPP_COMPARE(expr.asc(), "(17 + 4) ASC");
    SQLPP_COMPARE(expr.desc(), "(17 + 4) DESC");
    SQLPP_COMPARE(expr.order(sqlpp::sort_type::asc), "(17 + 4) ASC");
    SQLPP_COMPARE(expr.order(sqlpp::sort_type::desc), "(17 + 4) DESC");
  }
  return 0;
}
