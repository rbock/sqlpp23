[**\< Index**](/docs/README.md)

# Change log

## 0.67
- `core/name/create_name_tag.h` is now a stand-alone header
- Instead of `cte(sqlpp::alias::a)`, you now have to call `sqlpp::cte(sqlpp::alias::a)`

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

