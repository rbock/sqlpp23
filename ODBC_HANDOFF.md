# ODBC connector — session handoff

Context document for continuing work on the sqlpp23 ODBC connector on
another machine. Written after the initial implementation on macOS
(2026-07-14). Tell Claude "Read ODBC_HANDOFF.md" at the start of the next
session. This file can be deleted once the cross-platform testing is done.

## State

- Complete new ODBC connector in `include/sqlpp23/odbc/`, enabled with
  `-DBUILD_ODBC_CONNECTOR=ON` (CMake's built-in `FindODBC`).
- All 136 tests pass on macOS (unixODBC 2.3.14, Homebrew); the 8 ODBC tests
  also pass *live* against the SQLite ODBC driver (`brew install sqliteodbc`).
- The `BUILD_WITH_MODULES` variant of `modules/sqlpp23.odbc.cppm` is
  **unvalidated**: no `clang-scan-deps` on the Mac (this equally affects the
  existing core/sqlite3 modules). Worth trying on Windows with MSVC.
- Docs: `docs/connectors/odbc.md` (includes how to run the tests).

## File map

```
include/sqlpp23/odbc/
  odbc.h                        umbrella header
  cursor_result.h               result type (block cursor + SQLGetData streaming)
  prepared_statement.h          SQLPrepare/SQLBindParameter, param buffers
  to_sql_string.h               '?' params, {d}/{t}/{ts} escape sequences
  detail/diagnostics.h          throw_on_error / make_exception (SQLGetDiagRec)
  database/connection.h         connection_base, transactions, isolation levels
  database/connection_config.h  variant<data_source, connection_string> + tuning
  database/connection_handle.h  env (ODBC 3.8) + dbc RAII, SQLConnect/SQLDriverConnect
  database/exception.h          exception with sql_state() + native_error_code()
  database/connection_pool.h, database/serializer_context.h
modules/sqlpp23.odbc.cppm
tests/odbc/                     serialize (offline), usage (ConnectionConfig, Statements)
tests/include/sqlpp23/tests/odbc/  tables.h (SQLite-flavored DDL!), helpers
```

## Key design decisions

- **One result class** (`cursor_result_t`), two fetch strategies decided at
  first `next()` after the row's `bind_field` calls record the column plan:
  - **Block cursor**: all columns bindable → `SQLBindCol` column-wise arrays,
    `SQL_ATTR_ROW_ARRAY_SIZE = connection_config::row_array_size` (default 64),
    `SQL_ATTR_ROWS_FETCHED_PTR` (heap-allocated so moves are safe). Handles
    drivers that lower/reject the rowset size.
  - **Streaming**: any blob column, any long-typed column (`SQL_LONGVARCHAR`,
    `SQL_WLONGVARCHAR`, `SQL_LONGVARBINARY`), unknown size, or text wider than
    `connection_config::max_bound_column_size` (default 4096; text budgeted
    ×4 for UTF-8 +1 NUL) → row-at-a-time, `SQLGetData` per column in
    increasing index order (the only pattern *every* driver supports),
    growing buffer with the standard truncation/`SQL_NO_TOTAL` loop.
  - All-or-nothing: mixing bound arrays and SQLGetData needs `SQLSetPos` /
    `SQL_GD_BLOCK`, which is driver-specific — deliberately avoided.
- Statement handles are `shared_ptr<void>`; a result from a prepared
  statement shares ownership. Result dtor does `SQL_CLOSE` + `SQL_UNBIND` and
  resets `ROWS_FETCHED_PTR`/`ROW_ARRAY_SIZE` so the prepared statement can be
  re-executed.
- Transactions: no `BEGIN` in ODBC — autocommit off + `SQLEndTran`; isolation
  via `SQL_ATTR_TXN_ISOLATION` (persists across transactions; documented).
- Parameters: scalars/chrono copied into per-index `parameter_data` (vector
  sized once at prepare from the serializer's param count — never resized,
  addresses are registered with the driver); text/blob bound by pointer into
  the core prepared statement's storage (same lifetime guarantee MySQL
  connector relies on). NULL = `SQL_C_DEFAULT`/`SQL_VARCHAR` + `SQL_NULL_DATA`.
- No `last_insert_id` (nothing portable in ODBC). No `constraints.h`
  (generic ODBC can't know backend capabilities at compile time).
- `escape()` doubles single quotes without needing a live connection — that's
  why serialize tests run offline (`SQLPP_COMPARE` uses an unconnected
  `connection`).

## Lessons learned (from live testing with sqliteodbc)

- **Do not trust `SQLDescribeCol` sizes for binary columns**: sqliteodbc
  reports every `blob` as `SQL_BINARY` size 255. Originally blobs were
  block-bound by declared size; a 100 KB blob tripped the truncation guard.
  Fix: blobs and long types always stream.
- sqliteodbc reports unbounded `TEXT` as `SQL_LONGVARCHAR` with fake size
  65536 → the long-type check catches it.
- sqliteodbc rounds bound **timestamp parameters to milliseconds** (literals
  and reads keep microseconds — reads return correct nanosecond `fraction`).
  `Statements.cpp` therefore uses a millisecond-precision test value.
- sqliteodbc supports block cursors (rowset sizes 3 and 64 verified).
- unixODBC quirk: attribute buffers for "32-bit" attributes are read into
  `SQLULEN`-sized variables defensively (some drivers write 64 bits).
- `assert`-style checks in `Statements.cpp` use a custom `expect` macro that
  throws (works in Release builds too).

## How to run the tests

```sh
cmake -S . -B build -DBUILD_ODBC_CONNECTOR=ON
cmake --build build
ctest --test-dir build -R odbc                  # offline part
# Live part (any ODBC driver; example: SQLite driver on macOS):
SQLPP_ODBC_CONNECTION_STRING="Driver=/opt/homebrew/lib/libsqlite3odbc.dylib;Database=/tmp/sqlpp23_odbc_test.db;" \
  ctest --test-dir build -R odbc
```

`tests/odbc/usage/Statements.cpp` is the integration test: it skips (returns
0) without `SQLPP_ODBC_CONNECTION_STRING`, otherwise asserts round trips of
all data types, NULLs, rowset boundaries, streaming, and transactions.

## TODO / next steps on Windows

- Windows has its own driver manager (`odbc32.lib`) — `find_package(ODBC)`
  should just work; no unixODBC needed.
- Test against real drivers, e.g.:
  - SQL Server: `Driver={ODBC Driver 18 for SQL Server};Server=...;Database=...;Uid=...;Pwd=...;TrustServerCertificate=yes;`
  - MySQL: `Driver={MySQL ODBC 9.x Unicode Driver};Server=...;Database=...;User=...;Password=...;`
  - PostgreSQL: `Driver={PostgreSQL Unicode};Server=...;Database=...;Uid=...;Pwd=...;`
  - SQLite: `Driver=SQLite3 ODBC Driver;Database=c:\path\test.db;`
- **The test DDL is SQLite-flavored** (`INTEGER PRIMARY KEY AUTOINCREMENT` in
  `tests/include/sqlpp23/tests/odbc/tables.h` / `tables.sql`) — needs
  per-backend DDL to run `Statements.cpp` against other databases.
- Things to watch on Windows / other drivers:
  - The connector uses the **narrow (ANSI) ODBC API**. On Windows the driver
    manager converts to the W-API using the ANSI code page — non-ASCII text
    may mangle unless the process code page is UTF-8. Possible follow-up:
    optional wide-char (`SQLWCHAR`) support.
  - MSVC: repo targets C++23; `not`/`and` alternative tokens are used
    throughout (core does the same) — needs `/permissive-` (default with
    recent MSVC standards flags).
  - SQL Server: `time` columns are `SQL_SS_TIME2` (driver-specific type) —
    check that binding `SQL_C_TYPE_TIME` still works; `datetimeoffset` is not
    supported by the connector.
  - Check `SQL_C_UBIGINT` support on each driver (used for unsigned bigint).
  - Verify the truncation guard doesn't fire on drivers that report character
    column sizes differently (we budget ×4 bytes/char for UTF-8).
- Consider CI for the ODBC build (e.g. Linux + unixODBC + sqliteodbc, and a
  Windows job).
- `BUILD_WITH_MODULES` build of `sqlpp23.odbc.cppm` still unvalidated.
