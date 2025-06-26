[**< Index**](/docs/README.md)

# TODOs

## Allow use as module
### And/or: Suppress export of symbols

Some compilers tend to export all the generated symbols, which is a bit annoying
in case of template-heavy libraries like sqlpp23 (leads to larger files and
longer compile/link/startup times, I believe). There are ways to suppress this
in most compilers, afaik.

## Need to document: select_as
select_ref_t is not fully type safe (but should offer reasonable protection).
The reason for this is to reduce the sheer size of the serialized type, e.g. in error messages.

## Need to document: cte_ref and friends
Not new, but also not documented before: Need to document that you need to be a bit careful with aliased CTEs as we use cte_ref in columns, from, and join.

## Need to document?
Note that in a sub select that is used as a value, we don't detect if a table is statically required but dynamically provided. This is because we do not have the full picture: The sub select could use tables from the enclosing query.

## EXTRACT function for date/time
See https://github.com/rbock/sqlpp11/issues/611

## Support for pragma tables
See https://github.com/rbock/sqlpp11/issues/553

## Async support

Obtain results in an asynchronous fashion, see #35, for instance.

[**< Index**](/docs/README.md)
