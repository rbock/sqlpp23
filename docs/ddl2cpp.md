[**\< Index**](/docs/README.md)

# Code generation

sqlpp23 requires C++ representations for the database tables you want interact
with. You can generate these table representations using the `sqlpp23-ddl2cpp` script.

## Generate ddl files

You need DDL files representing the tables you are interested in. Often, you
would have them for you project anyway, because you need them to set up
databases.

Otherwise, you can obtain them from your database backend. Here is an example of
how this could look like:

```
mysqldump --no-data MyDatabase > MyDatabase.sql

```

For detailed instructions refer to the documentation of your database.

## Generate C++ headers

Once you have the DDL files, you can create C++ headers or modules for them with provided
[sqlpp23-ddl2cpp](/scripts) script, e.g.

```
# generate header from ddl file(s)
scripts/sqlpp23-ddl2cpp \
    --path-to-ddl my_project/tables.ddl
    --namespace my_project
    --path-to-header my_project/tables.h
```

```
# generate module from ddl file(s)
scripts/sqlpp23-ddl2cpp \
    --path-to-ddl my_project/tables.ddl
    --namespace my_project
    --path-to-module my_project/tables.cppm
```

More details can be learned from `scripts/sqlpp23-ddl2cpp --help`.

[**\< Index**](/docs/README.md)
