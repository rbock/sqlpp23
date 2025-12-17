[**< Index**](/docs/README.md)

# Setup

In order to do anything really useful with the library, you will also need a
database backend and library to talk to it. The following backends are currently
supported:

- MySQL
- MariaDB
- SQLite3
- SQLCipher
- PostgreSQL

## Basic usage with CMake

The library supports two ways how it can be integrated with cmake.

You can find examples for both methods in the examples folder.

1. FetchContent (Recommended, no installation required)
1. FindPackage (installation required, see below)

Both methods will provide the following CMake targets

| Target | How to include | Description |
| ------ | -------------- | ----------- |
| sqlpp23::core | #include <sqlpp23/sqlpp23.h> | The core functionality, which is not connector-specific, as headers |
| sqlpp23::core_module | import sqlpp23::core; | The core functionality, which is not connector-specific, as a module |
| sqlpp23::sqlpp23 | #include <sqlpp23/sqlpp23.h> | A backwards-compatible alias of sqlpp23::core |
| sqlpp23::mariadb | #include <sqlpp23/mysql/mysql.h>[^1] | The MariaDB connector as headers |
| sqlpp23::mariadb_module | import sqlpp23::mariadb; | The MariaDB connector as a module |
| sqlpp23::mysql | #include <sqlpp23/mysql/mysql.h> | The MySQL connector as headers |
| sqlpp23::mysql_module | import sqlpp23::mysql; | The MySQL connector as a module |
| sqlpp23::postgresql | #include <sqlpp23/postgresql/postgresql.h> | The PostgreSQL connector as headers |
| sqlpp23::postgresql_module | import sqlpp23::postgresql; | The PostgreSQL connector as a module |
| sqlpp23::sqlcipher | #include <sqlpp23/sqlite3/sqlite3.h>[^2] | The SQLCipher connector as headers |
| sqlpp23::sqlcipher_module | import sqlpp23::sqlcipher; | The SQLCipher connector as a module |
| sqlpp23::sqlite3 | #include <sqlpp23/sqlite3/sqlite3.h> | The SQLite3 connector as headers |
| sqlpp23::sqlite3_module | import sqlpp23::sqlite3; | The SQLite3 connector as a module |

[^1]: The MariaDB connector re-uses the codebase of the MySQL connector. That's
why you use the MariaDB connector by including the MySQL headers.
[^2]: The SQLCipher connector re-uses the codebase of the SQLite3 connector.
That's why you use the SQLCipher connector by including the SQLite3 headers.

These targets will make sure all required dependencies are available, correctly
linked, include directories are set correctly and module interface files (if
using the module targets) are added to your project sources.

## Build and install

> [!IMPORTANT]
> MSVC users will need to define the preprocessor macro `NOMINMAX`
> when using the mysql backend (mysql.h includes windows.h which defines `min`
> and `max` unless told not to).

**Note**: Depending on how you use the lib, you might not need to install it
(see Basic Usage).

Download and unpack the latest release from
https://github.com/rbock/sqlpp23/releases or clone the repository. Inside the
directory run the following commands:

```bash
cmake -B build <options>
cmake --build build --target install
```

The last step will build the library and install it system wide, therefore it
might need admins rights.

By default only the core library will be installed. To also install connectors
set the appropriate variable to `ON`:

- `BUILD_MYSQL_CONNECTOR`
- `BUILD_MARIADB_CONNECTOR`
- `BUILD_POSTGRESQL_CONNECTOR`
- `BUILD_SQLITE3_CONNECTOR`
- `BUILD_SQLCIPHER_CONNECTOR`

The library will check if all required dependencies are installed on the system.
If connectors should be installed even if the dependencies are not yet available
on the system, set `DEPENDENCY_CHECK` to `OFF`.

Example: Install the core library, sqlite3 connector and postgresql connector.
Don’t check if the dependencies are installed and don’t build any tests:

```bash
cmake -B build -DBUILD_POSTGRESQL_CONNECTOR=ON -DBUILD_SQLITE3_CONNECTOR=ON -DDEPENDENCY_CHECK=OFF -DBUILD_TESTING=OFF
cmake --build build --target install
```

[**< Index**](/docs/README.md)
