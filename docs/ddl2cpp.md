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

## Command-line options

**Help**
| Option | Required | Default | Description |
| ------ | -------- | ------- | ----------- |
| -h or --help | No[^1] | | Display help and exit. |

**Required parameters for code generation**
| Option | Required | Default | Description |
| ------ | -------- | ------- | ----------- |
| --path-to-ddl PATH_TO_DDL ... | Yes | | One or more pathnames of input DDL files. |
| --namespace NAMESPACE | Yes | | C++ namespace of the generated database model. |

**Paths**
| Option | Required | Default | Description |
| ------ | -------- | ------- | ----------- |
| --path-to-header PATH_TO_HEADER | No[^3] | | Output pathname of the generated C++ header. |
| --path-to-header-directory PATH_TO_HEADER_DIRECTORY | No[^3] | | Output Directory for the generated C++ headers |
| --path-to-module PATH_TO_MODULE | No[^2][^3] | | Output pathname of the generated C++ module. |
| --path-to-custom-type-mapping PATH_TO_CUSTOM_TYPE-MAPPING | No | | Input pathname of a CSV file mapping SQL types or column names tp C++ types. |

**Additional options**
| Option | Required | Default | Description |
| ------ | -------- | ------- | ----------- |
| --module-name MODULE_NAME | No[^2] | | Name of the generated C++ module |
| --postgresql-schema SCHEMA | No | | Strip this schema prefix from PostgreSQL table names (e.g. `public`). Required when using `COMMENT ON COLUMN` annotations from a PostgreSQL dump (see [C++ type overrides](#c-type-overrides)). |
| --suppress-timestamp-warning | No | False | Don't display a warning when date/time data types are used. |
| --assume-auto-id | No | False | Treat columns called *id* as if they have a default auto-increment value. |
| --naming-style {camel-case,identity} | No | camel-case | Naming style for generated tables and columns. *camel-case* interprets *_* as word separator and translates table names to *UpperCamelCase* and column names to *lowerCamelCase*. *identity* uses table and column names as-is in generated code. |
| --generate-table-creation-helper | No | False | For each table in the database model create a helper function that drops and creates the table. |
| --use-import-sqlpp23 | No | False | Import sqlpp23 as module instead of including the header file. |
| --use-import-std | No | False | Import std as module instead of including the respective standard header files. |
| --custom-directives | No | | One or more custom lines to be added after the default include / import directives in the generated file. |

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
| 30 | Bad custom types. The specified [custom type mapping](/docs/custom_type_mapping.md) file is not valid. |
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
