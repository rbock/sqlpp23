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

## Custom data types

`sqlpp23-ddl2cpp` allows you to define aliases of existing data types, which can be useful if your DDL files use
SQL data types that are not supported natively by sqlpp23. In this case you can create a file mapping each of the
unsupported SQL data types to one of the natively supported base types and tell `sqlpp23-ddl2cpp` to use that
file through the `--path-to-custom-types` command-line option (see [Command-line options](#command-line-options)).

The custom types file is a CSV file in which fields can be optionally single or double-quoted. Each line
starts with a base type, followed by one or more custom types that are mapped to that base type. For example:
```
integral, MYINT, another_int, type2
floating_point, double, "my float", fpvalue, fp_too
```
The base type names follow closely the data types supported by sqlpp23:
- blob: Blob (binary large object).
- boolean: Boolean.
- date: Date (year + month + day of month).
- floating_point: Floating-point.
- integral: Integral (integer) data type. Booleans are not considered an integral type.
- serial: Integral (integer) value with auto-increment. Always has a default value, so it cannot be NULL.
- text: Text.
- time: Time of day.
- timestamp: Tmestamp (date + time).

The custom type names are case-insensitive just like the native SQL types.

## C++ type overrides

sqlpp23 supports user-defined C++ types as column data types (see [Custom types](/docs/recipes/custom_types.md)).
`sqlpp23-ddl2cpp` provides four ways to assign a C++ type to a specific column. All four can be combined;
when more than one applies to the same column the order of precedence (highest first) is:
`--path-to-cpp-types` > `COMMENT ON COLUMN` > MySQL `COMMENT` clause > inline SQL comment.

| Option | MySQL | PostgreSQL | SQLite3 |
|--------|:-----:|:----------:|:-------:|
| `--path-to-cpp-types` CSV file | ✓ | ✓ | ✓ |
| Inline `--` comment | ✓ | ✓ | ✓ |
| MySQL column `COMMENT` clause | ✓ | | |
| PostgreSQL `COMMENT ON COLUMN` | | ✓ | |

### Option 1 — CSV file (`--path-to-cpp-types`)

Pass a CSV file where each line maps a `table_name:column_name` pair to a fully-qualified C++ type:

```
tab_point:x,::myapp::XCoord
tab_point:y,::myapp::YCoord
```

Tell `sqlpp23-ddl2cpp` to use that file through the `--path-to-cpp-types` command-line option. It is an
error if a line in the file refers to a table or column that does not exist in the DDL input.

### Option 2 — Inline SQL comment

Place a `-- ... cpp_type:<type> ...` comment on the line immediately before the column definition.
The annotation can appear anywhere in the comment; surrounding text is ignored:

```sql
CREATE TABLE tab_point (
    -- cpp_type:point_id
    id bigint AUTO_INCREMENT PRIMARY KEY,
    -- this column holds cpp_type:XCoord (horizontal axis)
    x bigint NOT NULL,
    -- cpp_type:YCoord
    y bigint NOT NULL
);
```

This option works with all backends. When using the SQLite3 backend this and `--path-to-cpp-types`
are the only available annotation methods, since SQLite3 DDL does not include `COMMENT ON COLUMN`
statements or column `COMMENT` clauses.

### Option 3 — MySQL column `COMMENT` clause

When using the MySQL or MariaDB backend, the annotation can be embedded in the column's `COMMENT`
attribute. The `cpp_type:` keyword is extracted from the comment string wherever it appears:

```sql
CREATE TABLE tab_foo (
    `id`     bigint NOT NULL AUTO_INCREMENT,
    `int_n`  int    DEFAULT NULL COMMENT 'cpp_type:test::my_int',
    PRIMARY KEY (`id`)
);
```

### Option 4 — PostgreSQL `COMMENT ON COLUMN`

When using the PostgreSQL backend, annotations can be embedded in `COMMENT ON COLUMN` statements.
The `cpp_type:` keyword is extracted from the comment string wherever it appears:

```sql
COMMENT ON COLUMN public.tab_point.x IS 'x coordinate cpp_type:XCoord (horizontal axis)';
COMMENT ON COLUMN public.tab_point.y IS 'cpp_type:YCoord';
```

Use `--postgresql-schema` together with this option so that schema-qualified table names
(e.g. `public.tab_point`) are resolved correctly against the generated table definitions
(see [Command-line options](#command-line-options)).

### Include headers for custom C++ types

`sqlpp23-ddl2cpp` does not emit `#include` directives for the headers that define custom C++ types.
Add those includes manually to the generated file, or arrange for them to be included transitively.

## Command-line options

**Help**
| Option | Required | Default | Description | Before v0.67 |
| ------ | -------- | ------- | ----------- | ------------ |
| -h or --help | No[^1] | | Display help and exit. | Same |

**Required parameters for code generation**
| Option | Required | Default | Description | Before v0.67 |
| ------ | -------- | ------- | ----------- | ------------ |
| --path-to-ddl PATH_TO_DDL ... | Yes | | One or more pathnames of input DDL files. | First command-line argument |
| --namespace NAMESPACE | Yes | | C++ namespace of the generated database model. | Third command-line argument |

**Paths**
| Option | Required | Default | Description | Before v0.67 |
| ------ | -------- | ------- | ----------- | ------------ |
| --path-to-header PATH_TO_HEADER | No[^3] | | Output pathname of the generated C++ header. | Second command-line argument |
| --path-to-header-directory PATH_TO_HEADER_DIRECTORY | No[^3] | | Output Directory for the generated C++ headers | Second command-line argument + -split-tables. |
| --path-to-module PATH_TO_MODULE | No[^2][^3] | | Output pathname of the generated C++ module. | N/A |
| --path-to-custom-types PATH_TO_CUSTOM_TYPES | No | | Input pathname of a CSV file defining aliases of existing data types. | Same |
| --path-to-cpp-types PATH_TO_CPP_TYPES | No | | Input pathname of a CSV file mapping `table_name:column_name` pairs to fully-qualified C++ types (see [C++ type overrides](#c-type-overrides)). | N/A |

**Additional options**
| Option | Required | Default | Description | Before v0.67 |
| ------ | -------- | ------- | ----------- | ------------ |
| --module-name MODULE_NAME | No[^2] | | Name of the generated C++ module | N/A |
| --postgresql-schema SCHEMA | No | | Strip this schema prefix from PostgreSQL table names (e.g. `public`). Required when using `COMMENT ON COLUMN` annotations from a PostgreSQL dump (see [C++ type overrides](#c-type-overrides)). | N/A |
| --suppress-timestamp-warning | No | False | Don't display a warning when date/time data types are used. | -no-timestamp-warning |
| --assume-auto-id | No | False | Treat columns called *id* as if they have a default auto-increment value. | -auto-id |
| --naming-style {camel-case,identity} | No | camel-case | Naming style for generated tables and columns. *camel-case* interprets *_* as word separator and translates table names to *UpperCamelCase* and column names to *lowerCamelCase*. *identity* uses table and column names as-is in generated code. |  -identity-naming |
| --generate-table-creation-helper | No | False | For each table in the database model create a helper function that drops and creates the table. | N/A |
| --use-import-sqlpp23 | No | False | Import sqlpp23 as module instead of including the header file. | N/A |
| --use-import-std | No | False | Import std as module instead of including the respective standard header files. | N/A |
| --self-test | No[^1] | | Run parser self-test. | -test |

[^1]: Overrides every other option.
[^2]: To generate a C++ module, both --path-to-module and --module-name should be specified.
[^3]: Exactly one of --path-to-module, --path-to-header or --path-to-header-directory should be specified.

## Program exit code

The program follows the POSIX standard and exits with a zero code on success and a non-zero code on failure. Below is a full list of the currently supported exit codes:

| Exit code | Meaning |
| --------: | ------- |
| 0 | Success. The requested operation(s) were completed successfully. |
| 1 | Bad command-line arguments. The specified command-line arguments or their combination was invalid. |
| 10 | DDL execution error. The input DDL file(s) were valid syntactically but had a semantic error, e.g. duplicate table name, duplicate column name, column using an unknown data type ([custom data types](#custom-data-types) might help you in this case), etc. |
| 20 | DDL parse error. At least one of the specified DDL input file(s) has invalid syntax. |
| 30 | Bad custom types. The specified [custom data types](#custom-data-types) file is not valid. |
| 31 | Bad C++ types. The specified [C++ type overrides](#c-type-overrides) file is not valid, or references a table or column that does not exist in the DDL input. |
| Other | OS-specific runtime error. While the program does not use these exit codes directly, some OSes may report other termination codes, that are not listed here, when the program fails to run or is terminated forcefully. |

Please note that the error codes are not set in stone and may change in the future. The only thing the program guarantees, is that a zero exit code means success and a non-zero code means *some kind* of error.

## Examples of program usage

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
