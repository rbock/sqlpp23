[**\< Index**](/docs/README.md)

# Supported compilers

As of September 2025 (version 0.67), this is still evolving, but all tests build and pass with modules using either

* clang++-20.1.2
* g++-15.2.0 or 15.3.0 (15.2.1 has a regression which prevents it from compiling sqlpp23 correctly)

# Building and installing with modules support

If you want to build the library with modules, configure the project using the `Ninja` generator and specify the `-DBUILD_WITH_MODULES=ON`, e.g.

```bash
cmake \
    -B build \
    -G Ninja \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_CXX_STANDARD=23 \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DBUILD_POSTGRESQL_CONNECTOR=ON \
    -DBUILD_SQLITE3_CONNECTOR=ON \
    -DBUILD_MYSQL_CONNECTOR=ON \
    -DBUILD_TESTING=ON \
    -DDEPENDENCY_CHECK=ON \
    -DBUILD_WITH_MODULES=ON \
    --fresh
```

After that, optionally, build the project tests and run them, e.g.

```bash
cmake --build build
cd build
ctest
cd ..
```

After that, in most cases, you will install the library, e.g.

```bash
cmake --install build
```

The final install step will, among other things, copy the module interface files from [modules directory](/modules) to `<prefix>/modules/sqlpp23`.

# Using sqlpp23 modules in your projects

Since sqlpp23 is a header-only library, its module interface files are not precompiled and instead are installed as source code. It is up to the user to include the relevant module interface files into their project and then compile them as a part of their build process. The CMake config script for sqlpp23 defines a helper variable called `SQLPP23_MODULE_INTERFACE_DIR`, which contains the installation directory of the module interface files.

The following snippet of CMake code will find the `core` and `postgresql` components and then will add their interface files to the project.

```cmake
project (myapp CXX)
...
find_package (Sqlpp23 REQUIRED COMPONENTS PostgreSQL)
target_link_libraries (myapp PRIVATE sqlpp23::core sqlpp23::postgresql)
target_sources (
	myapp
	PRIVATE
		FILE_SET CXX_MODULES
		BASE_DIRS "${SQLPP23_MODULE_INTERFACE_DIR}"
		FILES
			"${SQLPP23_MODULE_INTERFACE_DIR}/sqlpp23.core.cppm"
			"${SQLPP23_MODULE_INTERFACE_DIR}/sqlpp23.postgresql.cppm"
)
...
```

After the module interface files have been added to your project, you can use `import` instead of `#include`. For example this code

```c++
#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/postgresql/postgresql.h>
```

Can be replaced with

```c++
import sqlpp23.core;
import sqlpp23.postgresql;
```

[**\< Index**](/docs/README.md)
