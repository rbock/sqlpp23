[**\< Connectors**](/docs/connectors.md)

# DuckDB connector

The DuckDB connector uses [DuckDB's C API](https://duckdb.org/docs/stable/clients/c/api) and lives in
`include/sqlpp23/duck_db`. The namespace is `sqlpp::duck_db`.

> [!NOTE]
> The connector directory is spelled `duck_db` (as in sqlpp11), while the CMake component and the
> package are called `DuckDB`. `cmake/modules/FindDuckDB.cmake` provides the `duckdb::duckdb`
> target either from a DuckDB CMake package or from a prebuilt release archive.

## Creating a connection

```c++
// Create a connection configuration. The default is an anonymous in-memory
// database.
auto config = std::make_shared<sqlpp::duck_db::connection_config>();
config->path_to_database = "/path/to/database.duckdb";

// DuckDB's own configuration options can be passed along.
config->options = {{"threads", "4"}};

// Create a connection
sqlpp::duck_db::connection db;
db.connect_using(config); // This can throw an exception.
```

`sqlpp::duck_db::connection_base::library_version()` returns the version of the DuckDB library the
program is linked against.

### Sharing a database between connections

A DuckDB *file* can only be opened once per process. The connector therefore keeps one
`duckdb_database` instance per path and reference counts it, so several sqlpp23 connections (and
therefore connection pools) can share the same database.

`:memory:` is not shared: every connection gets its own private anonymous database, which is DuckDB's
documented behaviour. Connection pools that need shared data must use a file.

## Parameter placeholders

DuckDB prepares plain `?` placeholders and binds them by 1-based position. This matches the order in
which sqlpp23 walks its parameter list, so no extra care is needed:

```c++
auto ps = db.prepare(select(tab.intN).from(tab).where(tab.intN == parameter(tab.intN)));
ps.parameters.intN = 7;
for (const auto& row : db(ps)) { /* ... */ }
```

## `insert_or_*`

DuckDB parses `INSERT OR REPLACE` and `INSERT OR IGNORE` as shorthands for the corresponding
`ON CONFLICT` clause, so both are available:

```c++
db(sqlpp::duck_db::insert_or_replace().into(tab).set(tab.textNnD = "test"));
db(sqlpp::duck_db::insert_or_ignore().into(tab).set(tab.textNnD = "test"));
```

## `returning`

`insert_into`, `update` and `delete_from` support `RETURNING`. Note that a statement with a
`RETURNING` clause produces a result set, not an `insert_result`/`command_result`:

```c++
for (const auto& row : db(sqlpp::duck_db::insert_into(foo)
                              .set(foo.textNnD = "test")
                              .returning(foo.id, foo.textNnD))) {
  // use row.id and row.textNnD
}
```

## Last insert id

DuckDB has **no** `last_insert_rowid()`. The connector therefore cannot provide the generated id
after a plain insert: `insert_result::last_insert_id` and `connection_base::last_insert_id()` are 0
in that case, and the supported way of obtaining generated values is `RETURNING`:

```c++
for (const auto& row : db(sqlpp::duck_db::insert_into(foo)
                              .set(foo.textNnD = "test")
                              .returning(foo.id))) {
  // row.id is the generated value
}
```

If a table needs an auto-incrementing column, use a sequence:

```sql
CREATE SEQUENCE seq_tab_foo_id START 1;
CREATE TABLE tab_foo (
  id BIGINT PRIMARY KEY DEFAULT nextval('seq_tab_foo_id'),
  ...
);
```

## `on_conflict`

`ON CONFLICT ... DO NOTHING` and `ON CONFLICT ... DO UPDATE` are supported:

```c++
db(sqlpp::duck_db::insert_into(foo)
       .set(foo.id = 1, foo.textNnD = "test")
       .on_conflict(foo.id)
       .do_nothing());

db(sqlpp::duck_db::insert_into(foo)
       .set(foo.id = 1, foo.textNnD = "test")
       .on_conflict(foo.id)
       .do_update(foo.textNnD = "other"));
```

## Attaching another database

```c++
sqlpp::duck_db::connection_config other;
other.path_to_database = "/path/to/other.duckdb";
auto schema = db.attach(other, "other");
```

## Transactions

`start_transaction`, `commit_transaction` and `rollback_transaction` are available, and a transaction
that goes out of scope without being committed is rolled back.

> [!NOTE]
> DuckDB uses snapshot isolation for all connections and does not expose a configurable
> per-connection isolation level through its C API. `get_default_isolation_level()` therefore always
> returns `sqlpp::isolation_level::serializable`, and `set_default_isolation_level()` accepts only
> `serializable` and `undefined`; every other value throws.

## Exceptions

In exceptional situations an `sqlpp::duck_db::exception` is thrown. The native error state can be
obtained through `error_code()`:

```c++
try {
  db(select(sqlpp::verbatim("nonsense").as(sqlpp::alias::a)));
} catch (const sqlpp::duck_db::exception& e) {
  std::println("Exception: {}, DuckDB state: {}", e.what(), e.error_code());
}
```

## `delete_from` ... `using`

`DELETE ... USING` is supported by DuckDB and by this connector:

```c++
db(sqlpp::duck_db::delete_from(foo).using_(bar).where(foo.id == bar.id));
```

## `for_update`

The SELECT locking clause (`FOR UPDATE`, `FOR SHARE`) is not supported and will fail to compile.
DuckDB parses it but rejects it with

```
Parser Error: SELECT locking clause is not supported!
```

## Multi-statement input

A prepared statement holds exactly one statement. Passing several statements separated by semicolons
is rejected with

```
DuckDB connector: Cannot execute multi-statements: >><statement><<
```

## Blobs

DuckDB has no `x'...'` hexadecimal blob literal. The connector serializes blobs as `'\xNN...'::BLOB`
and binds them through `duckdb_bind_blob`, so they round trip as `std::span<const uint8_t>`.

## Data types

| sqlpp23 | DuckDB type | notes |
| --- | --- | --- |
| `integral` | `BIGINT` | full `int64_t` range |
| `unsigned_integral` | `UBIGINT` | full `uint64_t` range, no int64 cast (unlike sqlite3) |
| `floating_point` | `DOUBLE` | `NaN`, `Infinity` and `-Infinity` are supported |
| `boolean` | `BOOLEAN` | |
| `text` | `VARCHAR` | read as `std::string_view` into the result buffer |
| `blob` | `BLOB` | read as `std::span<const uint8_t>` |
| `date` | `DATE` | |
| `time` | `TIME` | microseconds |
| `timestamp` | `TIMESTAMP` | microseconds |

## Differences to the sqlite3 connector

| Aspect | sqlite3 | DuckDB |
| --- | --- | --- |
| Parameter placeholder | `?1`, `?2`, ... | `?` (bound by position) |
| `last_insert_id()` | `last_insert_rowid()` | 0 unless `RETURNING` is used |
| Blob literal | `x'01FF'` | `'\x01\xff'::BLOB` |
| `unsigned_integral` | bound as int64 | native `UBIGINT` |
| NaN / Infinity | bound as `'NaN'` / `'Inf'` strings | native double values |
| `IS DISTINCT FROM` | `IS NOT` | `IS [NOT] DISTINCT FROM` (SQL standard) |
| `TRUNCATE` | `DELETE FROM` | `DELETE FROM` |
| `using` in `delete_from` | supported | supported |
| `for_update` (SELECT locking clause) | not restricted (fails at run time) | rejected at compile time (`assert_no_for_update_t`) |
| `with`, `returning`, `on_conflict`, `full_outer_join`, `right_outer_join`, `any`, cast to date/time | version dependent | always supported, no compile-time restriction |
| Attaching databases | `ATTACH '<file>' AS name` | `ATTACH '<file>' AS name` |
| In-memory sharing | per connection | per connection, file based sharing is reference counted |

## Building against DuckDB

The connector is header-only like the rest of sqlpp23; it needs `duckdb.h` at compile time and the
DuckDB library at link time.

With CMake:

```sh
cmake -S bundled/sqlpp23 -B build -DBUILD_DUCK_DB_CONNECTOR=ON -DDuckDB_DIR=/path/to/duckdb
```

Without an installed DuckDB package, pass the location of a prebuilt
[release archive](https://github.com/duckdb/duckdb/releases) (which contains `duckdb.h`,
`duckdb.lib` and `duckdb.dll`) through `DuckDB_DIR` or `DUCKDB_ROOT`; `FindDuckDB.cmake` also looks
in `third_party/duckdb/<version>` relative to the project.

[**\< Connectors**](/docs/connectors.md)
