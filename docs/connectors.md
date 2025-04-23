[**\< Index**](/docs/README.md)

# Database Connectors

Database connector classes connect your SQL code with the respective database backend.

They also make sure that statements are serialized in a way that the database comprehends. For instance,

```C++
foo.textN.is_distinct_from(std::nullptr);
```

will be serialized in different fashions depending on the connector you are using:

```C++
// mysql:
"NOT (tab_foo.id <=> NULL)"

// postgresql:
"tab_foo.id IS DISTINCT FROM NULL"

// sqlite3:
"tab_foo.id IS NOT NULL"
```

See the links below for details:

[MySQL & MariaDB](/docs/connectors/mysql.md)

## MySQL and MariaDB

## Postgresql

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

## Sqlite3 and SQLCipher

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

## Other connectors

If you want to use other databases, you would have to write your own connector.
Don't worry, it is not that hard, following the existing examples.

[**\< Index**](/docs/README.md)
