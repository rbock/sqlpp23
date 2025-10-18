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

**Additional options**
| Option | Required | Default | Description | Before v0.67 |
| ------ | -------- | ------- | ----------- | ------------ |
| --module-name MODULE_NAME | No[^2] | | Name of the generated C++ module | N/A |
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
