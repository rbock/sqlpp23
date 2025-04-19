[**\< Index**](README.md)

# With

The `with` clause provides [CTEs](tables.md) to select statements. The CTEs can
then be used as read-only tables in the statement, e.g. they can be provided in
the `from` clause of a [`select`](select.md) statement and their columns can be
selected or used in a `where` clause.

```c++
const auto x = cte(sqlpp::alias::x).as(select(foo.id).from(foo));

for (const auto& row : db(with(x) << select(x.id).from(x).where(x.id < 17))) {
  // ...
}
```

[**\< Index**](README.md)
