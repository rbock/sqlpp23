[**\< Connectors**](/docs/connectors.md)

# Sqlite3 and SQLCipher connector

## Creating a connection

```c++
// Create a connection configuration.
auto config = std::make_shared<sqlpp::sqlite3::connection_config>();
config->path_to_database = ":memory:";
config->flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;

// Create a connection
sqlpp::sqlite3::connection db;
db.connect_using(config); // This can throw an exception.
```

See also the [logging documentation](/docs/logging.md).

## `insert_or_*`

The sqlite3 connector offers

- `insert_or_replace`
- `insert_or_ignore`

These can be used like this:

```c++
auto i = db(sqlpp::sqlite3::insert_or_replace().into(tab).set(
    tab.textNnD = "test", dynamic(true, tab.boolN = true)));
```

## `returning`

`insert_into`, `update`, and `delete_from` support the `returning` clause to return one or more columns from affected rows, for instance:

```c++
for (const auto& row :
     db(sqlpp::sqlite3::update(foo)
            .set(foo.intN = 0, foo.doubleN = std::nullopt)
            .returning(foo.textNnD, dynamic(maybe, foo.intN)))) {
  // use row.textNnD
  // use row.intN
}
```

## `on_conflict` ... `do_nothing`

The connector supports `ON CONFLICT ... DO NOTHING` with zero or more conflict targets, e.g.

```c++
// No conflict targets
sql::insert_into(foo).default_values().on_conflict().do_nothing());

// One or maybe two conflict targets
db(sql::insert_into(foo)
       .default_values()
       .on_conflict(foo.id)
       .do_nothing());
```

> [!NOTE]
> sqlpp23 does not understand constraints. It has no way of verifying whether the conflict targets are valid.

> [!NOTE]
> I have not figured out a valid scenario with more than one conflict target for sqlite3.
> Let me know if you have one.

## `on_conflict` ... `do_update` ... [`where` ...]

The connector supports `ON CONFLICT ... DO UPDATE ... WHERE` with zero or more conflict targets, one or more update assignments and optional `where` clause, e.g.

```c++
for (const auto& row : db(sql::insert_into(foo)
                              .default_values()
                              .on_conflict(foo.id)
                              .do_update(foo.intN = 5,
          dynamic(maybe, foo.textNnD = "test bla"), foo.boolN = true)
                              .where(foo.intN == 2)
                              .returning(foo.textNnD))) {
  // use row.textNnD;
}
```

## `any`

This is not supported and will fail to compile.

## `delete_from` ... `using`

`using` is not supported and will fail to compile.

These are not supported and will fail to compile before version 3.39.0.

## `full_outer_join` and `right_outer_join`

These are not supported and will fail to compile before version 3.39.0.

## `with`

This is not supported and will fail to compile before version 3.8.0.

## Exceptions

In exceptional situations that yield an Sqlite3 error code, an `sqlpp::sqlite3::exception` will be thrown. The native
error code can be obtained through the `error_code` function, e.g.

```c++
try {
  auto db = sqlpp::sqlite3::connection(config);
  db(select(sqlpp::verbatim("nonsense").as(sqlpp::alias::a)));
} catch (const sqlpp::sqlite3::exception& e) {
  println("Exception: {}, Sqlite3 error code: {}", e.what(), e.error_code());
}
```

[**\< Connectors**](/docs/connectors.md)
