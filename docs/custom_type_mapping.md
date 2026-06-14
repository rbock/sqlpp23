[**\< Index**](/docs/README.md)

# Custom type mapping

[`sqlpp23-ddl2cpp`](/docs/ddl2cpp.md) natively knows how to map many SQL types to C++ types. However, if you want, you can specify
custom mappings using several mechanisms as documented below.

Custom mappings can use the types provided by sqlpp23:

- `::sqlpp::blob`: Blob (binary large object).
- `::sqlpp::boolean`: Boolean.
- `::sqlpp::date`: Date (year + month + day of month).
- `::sqlpp::floating_point`: Floating-point.
- `::sqlpp::integral`: Integral (integer) data type. Booleans are not considered an integral type.
- `::sqlpp::serial`: Integral (integer) value with auto-increment. Always has a default value, so it cannot be NULL.
- `::sqlpp::text`: Text.
- `::sqlpp::time`: Time of day.
- `::sqlpp::timestamp`: Tmestamp (date + time).

In addition, sqlpp23 supports user-defined C++ types as column data types (see [Custom types](/docs/recipes/custom_types.md)).

## Mapping priorities

All mechanisms can be used in parallel. They are applied in the following order:

* sqlpp23 default mapping (lowest priority)
* CSV: SQL type -> C++ type
* CSV: column name -> C++ type
* CSV: table.column name -> C++ type
* CSV: schema.table.column -> C++ type
* comment annotations of any kind (these must not contradict each other)


## CSV file

The `--path-to-custom-type-mapping` command-line option (see [Command-line options](#command-line-options)) specifies the
path to a CSV file. Each line contains three comma-separated fields

1. `row_type`: This can be either `type` or `column`.
2. `entity`: If `row_type == type`, this is an SQL type. Otherwise this is a column name with or without table qualification, e.g. `bar` or `tab_foo.bar`.
3. `cpp_type`: This is the C++ type to represent the specified entity, e.g. `::sqlpp::integral` or `::some::uuid`.

For instance:

```CSV
type, SPECIAL INT, ::sqlpp::integral
type, UUID, ::some::uuid
column, first_name, ::my_own::string
column, tab_employee.first_name, ::sqlpp::text
column, internal.tab_employee.first_name, ::corp::string
```

## Inline SQL comment

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

This option works with all backends.

## MySQL column `COMMENT` clause

When using the MySQL or MariaDB backend, the annotation can be embedded in the column's `COMMENT`
attribute. The `cpp_type:` keyword is extracted from the comment string wherever it appears:

```sql
CREATE TABLE tab_foo (
    `id`     bigint NOT NULL AUTO_INCREMENT,
    `int_n`  int    DEFAULT NULL COMMENT 'cpp_type:test::my_int',
    PRIMARY KEY (`id`)
);
```

## PostgreSQL `COMMENT ON COLUMN`

When using the PostgreSQL backend, annotations can be embedded in `COMMENT ON COLUMN` statements.
The `cpp_type:` keyword is extracted from the comment string wherever it appears:

```sql
COMMENT ON COLUMN public.tab_point.x IS 'x coordinate cpp_type:XCoord (horizontal axis)';
COMMENT ON COLUMN public.tab_point.y IS 'cpp_type:YCoord';
```

Use `--postgresql-schema` together with this option so that schema-qualified table names
(e.g. `public.tab_point`) are resolved correctly against the generated table definitions
(see [sqlpp23-ddl2cpp](/docs/ddl2cpp.md)).

## Include headers for custom C++ types

Use the `--custom-directives` option to add custom `#include` or `import` lines to the generated file.

[**\< Index**](/docs/README.md)

