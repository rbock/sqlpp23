[**\< Connectors**](/docs/connectors.md)

# Sqlite3 and SQLCipher connector

## Creating a connection

```c++
// Create a connection configuration.
auto config = std::make_shared<sqlpp::sqlite3::connection_config>();
config->path_to_database = ":memory:";
config->flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
config->debug = true; // Will log a lot of debug output.

// Create a connection
sqlpp::sqlite3::connection db;
db.connect_using(config); // This can throw an exception.
```

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

## `any`

This is not supported and will fail to compile.

## `delete_from` ... `using`

`using` is not suppored and will fail to compile.

These are not supported and will fail to compile before version 3.39.0.

## `full_outer_join` and `right_outer_join`

These are not supported and will fail to compile before version 3.39.0.

## `with`

This is not supported and will fail to compile before version 3.8.0.

[**\< Connectors**](/docs/connectors.md)
