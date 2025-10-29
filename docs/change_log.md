[**\< Index**](/docs/README.md)

# Change log

## 0.68

- deprecate result_row_t::as_tuple
- new as_tuple(const result_row_t&)
- new get_sql_name_tuple(const result_row_t&), #72
- sqlpp23-ddl2cpp changes:
  - command-line option `--path-to-datatype-file` was renamed to `--path-to-custom-types`
  - if an error is found in the custom types file, the program exits with error code 30
  - base types in the custom types file should use `snake_case`, although the old `CamelCase` is still supported for backwards compatibility
  - renamed some base types in custom types file to match the corresponding sqlpp23 data types:
    - integer -> integral
    - date_time -> timestamp
  - documented the format of the custom types file
  - recognize and process "ALTER TABLE...ALTER COLUMN...SET DEFAULT" commands, which improves processing of pg_dump output scripts.

## 0.67

Module support

- Basic support for modules, see [docs](/docs/modules.md), #30
- `core/name/create_name_tag.h` is now a stand-alone header, #30
- Revamped commandline switches for `sqlpp23-ddl2cpp`, see [docs](/docs/ddl2cpp.md)
- Instead of `cte(sqlpp::alias::a)`, you now have to call `sqlpp::cte(sqlpp::alias::a)`
- Support comparison member functions for comparison expressions, e.g. you can now order by comparison results, #39
- order_by allows duplicate expressions, #39
- Install modules into `<prefix>/modules/sqlpp23`, #58

## 0.66

First release of sqlpp23

It is tagged 0.66 as it is a continuation from sqlpp11-0.65.

Major differences for library users are documented in
docs/differences_to_sqlpp11.md

Beyond that, the switch to C++23 makes the library easier to maintain,
easier to extend, and (I think) easier to read. One of the nicest
features of C++23 in this regard is "Deducing this" (P0847R7) as it
makes CRTP significantly simpler to write and reason about.

See for instance

- statement_t: include/sqlpp23/core/query/statement.h
- all clauses: include/sqlpp23/core/clause/*.h

[**\< Index**](/docs/README.md)

