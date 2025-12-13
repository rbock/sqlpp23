# sqlpp23

sqlpp23 is a type-safe embedded domain specific language for SQL queries and results in C++.
It allows you to write SQL in the form of C++ expressions:

  * Tables, columns, result fields are represented as structs or data members
  * Statements and clauses are constructed by functions
  * Your IDE can help you write SQL in C++
  * Your compiler will find a lot of typical mistakes long before they hit integration tests or even production, e.g.
    * typos
    * comparing apples to oranges
    * a missing table in `from()`
    * missing a non-default column in `insert_into()`
    * selecting a mix of aggregates and non-aggregates
    * differences in SQL dialects from one database backend to the next, see below

sqlpp23’s core is vendor-neutral.

Specific traits of databases (e.g. unsupported or non-standard features) are handled by connector libraries.
Connector libraries can inform you and your IDE of missing features at compile time.
They also interpret expressions specifically where needed.
For example, the connector could use the `operator||` or the `concat` method for string concatenation without your being required to change the statement.

Connectors for MariaDB, MySQL, PostgreSQL, sqlite3, sqlcipher are included in this repository.

Documentation is found in [docs](/docs/README.md).

If you are coming from [sqlpp11](https://github.com/rbock/sqlpp11), you might be interested in [differences](docs/differences_to_sqlpp11.md).

## Examples:

Let's assume we have a database connection object `db` and a table object `foo` representing something like

```SQL
CREATE TABLE foo (
    id bigint NOT NULL,
    name varchar(50),
    hasFun bool NOT NULL
);
```

[insert](/docs/insert.md)
```C++
db(insert_into(foo).set(foo.id = 17, foo.name = "bar", foo.hasFun = true));
```

[update](/docs/update.md)
```C++
db(update(foo).set(foo.name = std::nullopt).where(foo.name != "nobody"));
```

[delete](/docs/delete.md)
```C++
db(delete_from(foo).where(not foo.hasFun));
```

[select](/docs/select.md)
```C++
// selecting zero or more results, iterating over the results
for (const auto& row : db(select(foo.id, foo.name, foo.hasFun)
                          .from(foo)
                          .where(foo.id > 17 and foo.name.like("%bar%"))))
{
  std::cout << row.id << '\n';                    // int64_t
  std::cout << row.name.value_or("NULL") << '\n'; // std::optional<std::string_view>
  std::cout << row.hasFun() << '\n';              // bool
}
```

[database specific](/docs/connectors.md)
```C++
  for (const auto &row : db(sql::insert_into(foo)
                                .set(foo.id = 7, foo.name = std::nullopt, foo.hasFun = false)
                                .on_conflict(foo.id)
                                .do_update(foo.name = "updated")
                                .where(foo.id > 17)
                                .returning(foo.name))) {
    std::cout << row.name.value_or("NULL") << '\n';
  }
```

## Requirements:

__Compiler:__
sqlpp23 makes use of C++23 and requires a recent compiler and standard library.

If you use the library without modules, the following compiler versions are known to be sufficient:

* clang: 20.1 (both libstdc++ and libc++ work)
* gcc: 14.2
* MSVC: 19.44.35219

For modules, please check out [/docs/modules.md](/docs/modules.md).

## License:

sqlpp23 is distributed under the [BSD 2-Clause License](https://github.com/rbock/sqlpp23/blob/main/LICENSE).

## Status:
[![Build status](https://ci.appveyor.com/api/projects/status/9kyafm5p1xq5j0ax/branch/main?svg=true)](https://ci.appveyor.com/project/rbock/sqlpp23/branch/main)

## Getting involved:

Feature requests, bug reports, contributions to code or documentation are most welcome.

  * Issues at https://github.com/rbock/sqlpp23/issues
  * email at rbock at eudoxos dot de

