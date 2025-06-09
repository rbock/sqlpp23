[**\< Index**](/docs/README.md)

# Differences between sqlpp11 and sqlpp23

This is a (probably incomplete) list of differences as of June 2025.

If seems a bit dry, follow the links to examples.

| % | sqlpp11 | sqlpp23 |
| :------------- | :------------- | :----- |
| [**IDE code completion**](/docs/tony_tables/ide.md) | | |
| statement data members | all public | fewer and private (less noisy) |
| prepared statement data members | all public | *only* API public (less noisy) |
| connection functions | all public | *only* API public (less noisy) |
| | | |
| **Data types** | | |
| nullable values | `sqlpp::value_or_null` | `std::optional` |
| `NULL` | `sqlpp::null` | `std::nullopt` |
| date and time | `day_point`, `time_point`, and `time_of_day` are prepresented as `sqlpp::chrono::day_point`, `sqlpp::chrono::microsecond_point`, and `std::chrono::microseconds` | `date`, `timestamp`, `time` are represented as `std::chrono::sys_days`, `std::chrono::sys_time<std::chrono::microseconds>`, and `std::chrono::microseconds` |
| | | |
| **Result row** | | |
| result fields | specializations of the `result_field_t` template | standard types, like `int64_t`, `std::optional<int64_t>`, or `std::string_view` |
| `TEXT` result fields | convertible to `std::string` | `std::string_view` |
| `BLOB` result fields | convertible to `std::vector<uint8_t>` | `std::span<uint8_t>` |
| programmatic access | `for_each_field` | `result_row_t::as_tuple()` |
| | | |
| [**Names**](/docs/tony_tables/names.md) | | |
| creating names | `SQLPP_ALIAS_PROVIDER` | `SQLPP_CREATE_NAME_TAG` |
| creating quoted names | `SQLPP_QUOTED_ALIAS_PROVIDER` | `SQLPP_CREATE_QUOTED_NAME_TAG` |
| | | |
| [**Dynamic queries**](/docs/tony_tables/dynamic_queries.md) | | |
| clauses | separate calls to add dynamic parts with very few compile time checks | directly embedded in statement using `dynamic()` with many compile time checks |
| `where` conditions | dynamic `and` supported without nesting | dynamic `and` and `or` supported at any nesting level |
| result fields | dynamic result fields in `std::map<std::string, std::string>` | correctly typed and named data members of result rows |
| | | |
| **Database connection** | | |
| connect | `connectUsing` | `connect_using` |
| `operator()` | executes statements, but not strings | executes statements and strings |
| `execute` | executes strings and non-select statements | *dropped* |
| `query` | executes select statements | *dropped* |
| exceptions | many individual exception classes for postgresql, but not for other connectors | exception classes for each connector to transport native error codes |
| | | |
| [**Constraints**](/docs/tony_tables/constraints.md) | | |
| read-only columns  | e.g. for auto-increment | for generated columns (`GENERATED`) |
| required `where`  | in `select`, `update`, `remove` | *dropped* |
| `unconditionally()`  | to explicitly omit `where` or `on` condition in joins | *dropped* |
| | | |
| [**Clauses**](/docs/tony_tables/clauses.md) | | |
| `DELETE FROM`  | `remove_from` | `delete_from` |
| `LIMIT` & `OFFSET`  | require unsigned argument | any integer argument |
| `TRUNCATE`  | N/A | `truncate` |
| `WITH`  | connect `with` with statement using `operator()` | connect `with` with statement using `operator<<` |
| custom queries  | `sqlpp::custom_query` | clauses can be concatenated using `operator<<` |
| | | |
| **Functions** | | |
| `COALESCE` | N/A | `coalesce(<one or more arguments>)` |
| `COUNT(*)` | N/A | `count(sqlpp::star)` |
| `SOME` | `some` | *dropped* (use `any`) |
| | | |
| [**Operators**](/docs/tony_tables/operators.md) | | |
| `CASE` | `case_when.then().else_()` supports a single `when`/`then` pair. Data types of `then` and `else_` must be comparable. `then` does not support `sqlpp::null` | Support for multiple `when`/`then` pairs. Data types of `then`s and `else` must be same (plus/minus optional). `then` supports `std::nullopt` if at least one `then` argument or the `else_` argument has a data type. |
| `IN` | `in` requires `sqlpp::value_list` to pass `std::vector` | `something.in(my_vector)` |
| `NOT IN` | `not_in` requires `sqlpp::value_list` to pass `std::vector` | `something.not_in(my_vector)` |
| Unary `operator+()` | present | *dropped* |
| comparison with value or `NULL` | `is_equal_to_or_null` (does not work with parameters) | `is_distinct_from` and `is_not_distinct_from` |
| | | |
| **Sub queries** | | |
| `select(...).as(...)` | Could be table or value (depending on the context) | Always a table unless wrapped by `value()` |
| | | |
| [**Logging**](/docs/tony_tables/logging.md) | | |
| config.debug | Could control logging to `std::cerr` via a bool value in the connection's config | Can control which message categories are logged and control how hey are logged (you can provide a `std::function`). And you can turn off debug logging completely at compile time by defining `SQLPP23_DISABLE_DEBUG`. |
| | | |
| **Magic** | | |
| `IN` | `x.in(sqlpp::value_list(vector{}))` interpreted as `false` | *no magic*: `x IN ()` |
| `NOT IN` | `x.not_in(sqlpp::value_list(vector{}))` interpreted as `true` | *no magic*: `x NOT IN ()` |
| `operator+=` etc | `x += y` was translated into `x = x + y` | *no magic*  |
| aggregate functions | auto-named in `select` but not otherwise | require explicit names, e.g. max(id).as(sqlpp::alias::max_) |
| **Misc** | | |
| prepared statement parameters | `.params` | `.parameters` |
| `eval(db, expr)` | Convenience wrapper around `db(select(expr.as(a))).front().a` | *dropped* (could lead to dangling references, see `TEXT` and `BLOB`) |
| `ppgen` | Pre-processor code generation for tables | *dropped* |
| sqlite2cpp.py | ddl2cpp variant for sqlite3 | *dropped* |
| dynamic loading | Ability to load connector library dynamically | *dropped* (was unmaintained) |
| memory allocation | Multiple cases of double pointer indirections (wrapping a smart pointer with yet another smart pointer) | Removed all known double pointer indirections |

[**\< Index**](/docs/README.md)

