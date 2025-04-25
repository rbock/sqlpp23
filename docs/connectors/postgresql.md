[**\< Connectors**](/docs/connectors.md)

# Postgresql connector

## Creating a connection

```c++
// Create a connection configuration.
auto config = std::make_shared<sqlpp::postgresql::connection_config>();
config->user = "some_user";
config->database = "some_database";
config->debug = true; // Will log a lot of debug output.

// Create a connection
sqlpp::postgresql::connection db;
db.connect_using(config); // This can throw an exception.
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

TODO\
See [tests](/tests/postgresql/usage/InsertOnConflict.cpp)

## `on_conflict` ... `do_update` ... [`where` ...]

TODO\
See [tests](/tests/postgresql/usage/InsertOnConflict.cpp)

[**\< Connectors**](/docs/connectors.md)
