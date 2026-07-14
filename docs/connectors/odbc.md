[**\< Connectors**](/docs/connectors.md)

# ODBC connector

The ODBC connector uses ODBC 3.8 and works with any database that provides an
ODBC driver (through a driver manager like unixODBC, iODBC, or the Windows
ODBC driver manager).

Since ODBC only standardizes the API, not the SQL dialect, statements are
serialized in a portable fashion:

- parameters use unnumbered `?` markers,
- date/time literals use the ODBC escape sequences `{d '...'}`, `{t '...'}`,
  and `{ts '...'}`, which the driver translates for the database.

Anything beyond that (e.g. vendor-specific functions used via
`sqlpp::verbatim`) is passed through unchanged, so it is your responsibility
to only use SQL that your database understands.

## Creating a connection

A connection configuration holds exactly one way of connecting — either a
data source configured in the driver manager (e.g. `odbc.ini`) or a complete
connection string:

```c++
auto config = std::make_shared<sqlpp::odbc::connection_config>();

// Either: connect to a configured data source (SQLConnect)
config->source = sqlpp::odbc::data_source{
    .name = "my_dsn", .username = "user", .password = "password"};

// Or: connect using a connection string (SQLDriverConnect)
config->source = sqlpp::odbc::connection_string{
    "Driver=PostgreSQL Unicode;Server=localhost;Database=test;Uid=user;Pwd=password;"};

// Create a connection
sqlpp::odbc::connection db;
db.connect_using(config);  // This can throw an exception.
```

See also the [logging documentation](/docs/logging.md).

## Fetching multiple rows at once

Select results are read through an ODBC block cursor where possible: the
connector binds the result columns to fixed-size buffers and fetches
`connection_config::row_array_size` rows per driver round trip (default: 64).

This requires that the size of each row is known up front. Text columns
qualify if their declared maximum size fits into
`connection_config::max_bound_column_size` bytes per row (default: 4096,
budgeted with 4 bytes per character for UTF-8). If any column exceeds the
limit — or its size cannot be trusted, as with unbounded `TEXT` columns and
blobs — the result transparently falls back to fetching one row at a time
and streaming each value with `SQLGetData`, which has no size limit.

Setting `max_bound_column_size = 0` always streams results that contain
text or blob columns. This is useful for databases that do not enforce
declared column sizes (e.g. SQLite, where a `varchar(255)` column may hold
longer values).

```c++
auto config = std::make_shared<sqlpp::odbc::connection_config>();
config->row_array_size = 256;         // fetch up to 256 rows per round trip
config->max_bound_column_size = 65536; // allow wider text/blob columns
```

Setting `row_array_size = 1` disables multi-row fetch.

## Transactions

ODBC has no explicit `BEGIN`: `start_transaction` turns off autocommit and
`commit_transaction`/`rollback_transaction` end the transaction via
`SQLEndTran` and turn autocommit back on.

If you pass an isolation level to `start_transaction`, it is set through
`SQL_ATTR_TXN_ISOLATION` and remains in effect for subsequent transactions on
the same connection as well.

## Time of day precision

ODBC's `SQL_TIME_STRUCT` and time escape sequence have no sub-second
precision. Fractional seconds of `sqlpp::time` values are dropped when
serializing or binding parameters. Timestamps (`sqlpp::timestamp`) keep their
microsecond precision.

## `last_insert_id`

ODBC has no portable way to report the id of the last inserted row, so the
connector does not offer `last_insert_id`. Use a select with the appropriate
function of your database (e.g. `last_insert_rowid()`, `lastval()`,
`SCOPE_IDENTITY()`) if you need it.

## Running the tests

The ODBC tests live in `tests/odbc` and are built with
`-DBUILD_ODBC_CONNECTOR=ON`. The serialization and configuration tests run
without any ODBC driver:

```sh
cmake -S . -B build -DBUILD_ODBC_CONNECTOR=ON
cmake --build build
ctest --test-dir build -R odbc
```

The statement test (`sqlpp23.odbc.usage.Statements`) only checks compilation
by default. When `SQLPP_ODBC_CONNECTION_STRING` is set, it additionally runs
all statement types against the configured database and verifies the results,
e.g. with the serverless [SQLite ODBC driver](http://www.ch-werner.de/sqliteodbc/)
(`brew install sqliteodbc`):

```sh
SQLPP_ODBC_CONNECTION_STRING="Driver=/opt/homebrew/lib/libsqlite3odbc.dylib;Database=/tmp/sqlpp23_odbc_test.db;" \
  ctest --test-dir build -R odbc
```

The DDL in `tests/include/sqlpp23/tests/odbc/tables.h` is written for SQLite;
it may need adjustment for other databases.

## Exceptions

In exceptional situations the connector throws `sqlpp::odbc::exception` which
carries the message, SQLSTATE, and native error code of the diagnostic
records:

```c++
try {
  db(select(sqlpp::verbatim("nonsense").as(sqlpp::alias::a)));
} catch (const sqlpp::odbc::exception& e) {
  std::println("Exception: {}, SQLSTATE: {}, native error: {}", e.what(),
               e.sql_state(), e.native_error_code());
}
```

[**\< Connectors**](/docs/connectors.md)
