# Add tests that use query results as ranges
Add tests that use result_t in range operations. Also check if result_t::iterator satisfies the std::input_iterator concept.

# Allow use as module

# Need to document: aggregates
They must not be mixed in a select.

`group_by` accepts columns only (there is an escape hatch: `group_by_column`)
`select` requires either
- all selected columns are aggregate expressions (either an aggregate function or a group by column), or
- no selected column is an aggregate expression

If group_by is specified in the select, then all columns have to be aggregate expressions.

## Need to document: select_as
select_ref_t is not fully type safe (but should offer reasonable protection).
The reason for this is to reduce the sheer size of the serialized type, e.g. in error messages.

## Need to document: cte_ref and friends
Not new, but also not documented before: Need to document that you need to be a bit careful with aliased CTEs as we use cte_ref in columns, from, and join.

## Need to document?
Note that in a sub select that is used as a value, we don't detect if a table is statically required but dynamically provided. This is because we do not have the full picture: The sub select could use tables from the enclosing query.


## coalesce
https://github.com/rbock/sqlpp11/issues/453

# dynamic do_update?
https://github.com/rbock/sqlpp11/issues/493

# implement cast?
Implement cast, to support something like cast_as<sqlpp::day_point>(mt.startTime),
see https://github.com/rbock/sqlpp11/issues/606

# [PostgreSQL] on_conflict with more than one column
See https://github.com/rbock/sqlpp11/issues/586

# EXTRACT function for date/time
See https://github.com/rbock/sqlpp11/issues/611

# Upgrade case statement to allow for multiple when clauses.
See https://github.com/rbock/sqlpp11/issues/600

# Support for pragma tables
See https://github.com/rbock/sqlpp11/issues/553
