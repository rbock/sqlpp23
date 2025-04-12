[**\< Index**](README.md)

# Differences between sqlpp11 and sqlpp23

This is a (probably incomplete) list of differences as of April 2025.

## Data types

| % | sqlpp11 | sqlpp23 |
| :------------- | :------------- | :----- |
| **Data types** | | |
| nullable values | `sqlpp::value_or_null` | `std::optional` |
| `NULL` | `sqlpp::null` | `std::nullopt` |
| `TEXT` result fields | `std::string` | `std::string_view` |
| `BLOB` result fields | `std::vector<uint8_t>` | `std::span<uint8_t>` |
| | | |
| **Dynamic queries** | | |
| statements | separate calls to add dynamic parts with very few compile time checks | directly embedded in statement using `dynamic()` with many compile time checks |
| `where` conditions | dynamic `and` supported without nesting | dynamic `and` and `or` supported at any nesting level |
| result fields | dynamic result fields in `std::map<std::string, std::string>` | correctly typed and named data members of result rows |
| | | |
| **Constraints** | | |
| read-only columns  | e.g. for auto-increment | *dropped* |
| required `where`  | in `select`, `update`, `remove` | *dropped* |
| `unconditionally()`  | to explicitly omit `where` or `on` condition in joins | *dropped* |
| | | |
| **Clauses** | | |
| `DELETE FROM`  | `remove_from` | `delete_from` |
| `LIMIT` & `OFFSET`  | require unsigned argument | any integer argument |
| `TRUNCATE`  | N/A | `truncate` |
| custom queries  | `sqlpp::custom_query` | clauses can be concatenated using `operator<<` |
| | | |
| **Functions** | | |
| `COUNT(*)` | N/A | `count(sqlpp::star)` |
| `SOME` | `some` | *dropped* (use `any`) |
| aggregate functions | auto-named in `select` | require explicit names, e.g. max(id).as(sqlpp::alias::max_) |
| | | |
| **Operators** | | |
| Unary `operator+()` | present | *dropped* |
| comparison with value or `NULL` | `is_equal_to_or_null` (does not work with parameters) | `is_distinct_from` and `is_not_distinct_from` |
| | | |
| **Sub queries** | | |
| `select(...).as(...)` | Could be table or value (depending on the context) | Always a table unless wrapped by `value()` |
| | | |
| **Misc** | | |
| `eval(db, expr)` | Convenience wrapper around `db(select(expr.as(a))).front().a` | *dropped* (could lead to dangling references, see `TEXT` and `BLOB` |
| `value_list` | required for operator `in()` | *dropped* |
| `ppgen` | pre-processor code generation for tables | *dropped* |

# Aggregates and non-aggregates
They must not be mixed in a select.

`group_by` accepts columns only (there is an escape hatch: `group_by_column`)
`select` requires either
- all selected columns are aggregate expressions (either an aggregate function or a group by column), or
- no selected column is an aggregate expression

If group_by is specified in the select, then all columns have to be aggregate expressions.

## CTE
cte.join is now allowed.

## Need to document: select_as
select_ref_t is not fully type safe (but should offer reasonable protection).
The reason for this is to reduce the sheer size of the serialized type, e.g. in error messages.

## Need to document: cte_ref and friends
Not new, but also not documented before: Need to document that you need to be a bit careful with aliased CTEs as we use cte_ref in columns, from, and join.

## Introduced declare_group_by_column

## Need to document?
Note that in a sub select that is used as a value, we don't detect if a table is statically required but dynamically provided. This is because we do not have the full picture: The sub select could use tables from the enclosing query.

## dropped ppgn (cannot support it)

[**\< Index**](README.md)

