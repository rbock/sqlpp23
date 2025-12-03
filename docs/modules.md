[**\< Index**](/docs/README.md)

# Using modules

Instead of using sqlpp23 via include directives, you can also compile it into modules and then import them in your code, e.g.

```c++
import sqlpp23.core;
import sqlpp23.postgresql;
```

As of September 2025 (version 0.67), this is still evolving, but all tests build and pass with modules using either

* clang++-20.1.2
* g++-15.2.0 or 15.3.0 (15.2.1 has a regression which prevents it from compiling sqlpp23 correctly)

If you want to build tests with modules, call cmake with generator `ninja` and `-DBUILD_WITH_MODULES=ON`, e.g.

```bash
cmake ../ \
    -G Ninja \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_CXX_STANDARD=23 \
    -DBUILD_POSTGRESQL_CONNECTOR=ON \
    -DBUILD_SQLITE3_CONNECTOR=ON \
    -DBUILD_MYSQL_CONNECTOR=ON \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DBUILD_WITH_MODULES=ON \
    --fresh
```

The module sources can be found in the [modules directory](/modules). They get installed into `<prefix>/modules/sqlpp23`.

No compiled version of the modules will be installed. Your project will need to compile the modules sources itself.


[**\< Index**](/docs/README.md)
