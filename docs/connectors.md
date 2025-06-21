[**\< Index**](/docs/README.md)

# Database connectors

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

## Connectors provided by sqlpp23

[MySQL & MariaDB](/docs/connectors/mysql.md)

[PostgreSQL](/docs/connectors/postgresql.md)

[Sqlite3 and SQLCipher](/docs/connectors/sqlite3.md)

## Other connectors

If you want to use other databases, you would have to write your own connector.
Don't worry, it is not that hard, following the existing examples.

[**\< Index**](/docs/README.md)
