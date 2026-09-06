[**\< Index**](/docs/README.md)

# Differences between sqlpp11 and sqlpp23

Dropped schema-qualified tables (or rather, the idea is to make the schema part of the table name)
Reflection:
  - table definition
  - type_set
  - type_vector
  - result_set definition

constexpr:
  - flat_set -> type_set
  - exception (instead of wrapped static assert)

C++26 features

- Reflection
- constexpr flat_set
- constexpr vector
- constexor exception
- pack indexing
- structured binding pack
- consteval blocks
- template for

Currently still waiting for

- structural std::string_view


error quality:
at least one having-expression statically requires a table which is only known dynamically in the statement' != 'The having-clause statically requires table tab_bar which is only known dynamically in the statement'



[**\< Index**](/docs/README.md)

