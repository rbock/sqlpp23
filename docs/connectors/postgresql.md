[**\< Connectors**](/docs/connectors.md)

# Postgresql connector

## Creating a connection

```c++
// Create a connection configuration.
auto config = std::make_shared<sqlpp::postgresql::connection_config>();
config->user = "some_user";
config->database = "some_database";

// Create a connection
sqlpp::postgresql::connection db;
db.connect_using(config); // This can throw an exception.
```

See also the [logging documentation](/docs/logging.md).

## `delete_from`

The connector supports `using` and `returning` in `delete_from` statements, e.g.

```c++
sqlpp::postgresql::delete_from(tab)
    .using_(bar)
    .where(foo.bar_id == bar.id and
           bar.username == "johndoe")
    .returning(foo.id, dynamic(maybe, tab.blobN));
```

## `returning`

`insert_into`, `update`, and `delete_from` support the `returning` clause to return one or more columns from affected rows, for instance:

```c++
for (const auto& row :
     db(sqlpp::postgresql::update(foo)
            .set(foo.intN = 0, foo.doubleN = std::nullopt)
            .returning(foo.textNnD, dynamic(maybe, foo.intN)))) {
  // use row.textNnD
  // use row.intN
}
```

Similar to `select` columns, `returning` columns can be [`dynamic`](/docs/dynamic.md) and evaluate to `NULL` in case the dynamic condition is `false`.

## `on_conflict` ... `do_nothing`

The connector supports `ON CONFLICT ... DO NOTHING` with zero or more conflict targets, e.g.

```c++
// No conflict targets
sql::insert_into(foo).default_values().on_conflict().do_nothing());

// One or maybe two conflict targets
db(sql::insert_into(foo)
       .default_values()
       .on_conflict(foo.id, dynamic(false, foo.textNnD))
       .do_nothing());
```

> [!NOTE]
> sqlpp23 does not understand constraints. It has no way of verifying whether the conflict targets are valid.

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

## Exceptions

There are two types of exceptions specific to PostgreSQL in sqlpp23:

`sqlpp::postgresql::connection_exception` will be thrown in case of connection failures.

`sqlpp::postgresql::result_exception` will be thrown in case of statement execution or result evaluation failures.
Assuming a `PGresult` called `p`, this exception type provides `PQresultStatus(p)` via `status()` and `PQresultErrorField(p, PG_DIAG_SQLSTATE)` via `sql_state()`.

For instance:

```c++
try {
  auto db = sqlpp::postgresql::connection(config);
  db(select(sqlpp::verbatim("nonsense").as(sqlpp::alias::a)));
} catch (const sqlpp::postgresql::connection_exception& e) {
  println("Connection exception: {}, e.what());
} catch (const sqlpp::postgresql::result_exception& e) {
  println("Result exception: {}, status: {}, sql state: {}",
          e.what(), static_cast<int>(e.status()), e.sql_state());
}
```

[**\< Connectors**](/docs/connectors.md)
