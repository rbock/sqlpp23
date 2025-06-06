[**\< Connectors**](/docs/connectors.md)

# MySQL / MariaDB connector

## Creating a connection

```c++
// This will call `mysql_library_init` once (even if called multiple times).
// It will also ensure that `mysql_library_end()` is called during shutdown.
sqlpp::mysql::global_library_init();

// Create a connection configuration.
auto config = std::make_shared<sqlpp::mysql::connection_config>();
config->user = "some_user";
config->database = "some_database";

// Create a connection
sqlpp::mysql::connection db;
db.connect_using(config); // This can throw an exception.
```

See also the [logging documentation](/docs/logging.md).

## `update`

The connector supports `order_by` and `limit` in `update` statements, e.g.

```c++
sqlpp::mysql::update(tab)
    .set(tab.boolN = true)
    .order_by(tab.intN.desc())
    .limit(1);
```

## `delete_from`

The connector supports `using`, `order_by` and `limit` in `delete_from` statements, e.g.

```c++
sqlpp::mysql::delete_from(tab)
    .using_(bar)
    .where(foo.bar_id == bar.id and
           bar.username == "johndoe")
    .order_by(tab.intN.desc())
    .limit(1);
```

## No `full_outer_join`

The connector prevents the use of `full_outer_join` at compile time when you try to execute the statement, e.g

```c++
// This causes a static_assert:
// "MySQL: No support for full outer join"
db(select(foo.id, bar.name)
    .from(foo.full_outer_join(bar).on(foo.id == bar.id)));
```

## Exceptions

In exceptional situations that yield a MySQL error code, an `sqlpp::mysql::exception` will be thrown. The native
error code can be obtained through the `error_code` function, e.g.

```c++
try {
  auto db = sqlpp::mysql::connection(config);
  db(select(sqlpp::verbatim("nonsense").as(sqlpp::alias::a)));
} catch (const sqlpp::mysql::exception& e) {
  println("Exception: {}, MySQL error code: {}", e.what(), e.error_code());
}
```

[**\< Connectors**](/docs/connectors.md)

