[**\< Index**](README.md)

# Differences between sqlpp11 and sqlpp23

This is a (probably incomplete) list of differences as of April 2025.

If seems a bit dry, follow the links to examples.

| % | sqlpp11 | sqlpp23 |
| :------------- | :------------- | :----- |
| **IDE code completion** | | |
| statement data members | all public | fewer and private (less noisy) |
| connection functions | all public | *only* API public (less noisy) |
| | | |
| **Data types** | | |
| nullable values | `sqlpp::value_or_null` | `std::optional` |
| `NULL` | `sqlpp::null` | `std::nullopt` |
| **Result row** | | |
| result fields | specializations of the `result_field_t` template | standard types, like `int64_t`, `std::optional<int64_t>`, or `std::string_view` |
| `TEXT` result fields | convertible to `std::string` | `std::string_view` |
| `BLOB` result fields | convertible to `std::vector<uint8_t>` | `std::span<uint8_t>` |
| programmatic access | `for_each_field` | `result_row_t::as_tuple()` |
| | | |
| [**Names**](tony_tables/names.md) | | |
| creating names | `SQLPP_ALIAS_PROVIDER` | `SQLPP_CREATE_NAME_TAG` |
| creating quoted names | `SQLPP_QUOTED_ALIAS_PROVIDER` | `SQLPP_CREATE_QUOTED_NAME_TAG` |
| | | |
| [**Dynamic queries**](tony_tables/dynamic_queries.md) | | |
| clauses | separate calls to add dynamic parts with very few compile time checks | directly embedded in statement using `dynamic()` with many compile time checks |
| `where` conditions | dynamic `and` supported without nesting | dynamic `and` and `or` supported at any nesting level |
| result fields | dynamic result fields in `std::map<std::string, std::string>` | correctly typed and named data members of result rows |
| | | |
| **Executing statements** | | |
| `operator()` | executes statements, but not strings | executes statements and strings |
| `execute` | executes strings and non-select statements | *dropped* |
| `query` | executes select statements | *dropped* |
| | | |
| [**Constraints**](tony_tables/constraints.md) | | |
| read-only columns  | e.g. for auto-increment | *dropped* |
| required `where`  | in `select`, `update`, `remove` | *dropped* |
| `unconditionally()`  | to explicitly omit `where` or `on` condition in joins | *dropped* |
| | | |
| [**Clauses**](tony_tables/clauses.md) | | |
| `DELETE FROM`  | `remove_from` | `delete_from` |
| `LIMIT` & `OFFSET`  | require unsigned argument | any integer argument |
| `TRUNCATE`  | N/A | `truncate` |
| `WITH`  | connect `with` with statement using `operator()` | connect `with` with statement using `operator<<` |
| custom queries  | `sqlpp::custom_query` | clauses can be concatenated using `operator<<` |
| | | |
| **Functions** | | |
| `COUNT(*)` | N/A | `count(sqlpp::star)` |
| `SOME` | `some` | *dropped* (use `any`) |
| aggregate functions | auto-named in `select` | require explicit names, e.g. max(id).as(sqlpp::alias::max_) |
| | | |
| **Operators** | | |
| Unary `operator+()` | present | *dropped* |
| comparison with value or `NULL` | `is_equal_to_or_null` (does not work with parameters) | `is_distinct_from` and `is_not_distinct_from` |
| | | |
| **Sub queries** | | |
| `select(...).as(...)` | Could be table or value (depending on the context) | Always a table unless wrapped by `value()` |
| | | |
| **Misc** | | |
| `eval(db, expr)` | Convenience wrapper around `db(select(expr.as(a))).front().a` | *dropped* (could lead to dangling references, see `TEXT` and `BLOB`) |
| `value_list` | required for operator `in()` | *dropped* |
| `ppgen` | pre-processor code generation for tables | *dropped* |
| sqlite2cpp.py | ddl2cpp variant for sqlite3 | *dropped* |
| dynamic loading | ability to load connector library dynamically | *dropped* (was unmaintained) |

[**\< Index**](README.md)

