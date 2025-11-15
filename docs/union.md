[**\< Index**](/docs/README.md)

# Union

`UNION` statements combine SELECT statements, see [select](/docs/select.md).

There are two variants `union_all` and `union_distinct` which share the same synopsis, i.e. examples working with one of them would also work with the other.

Let's assume we have three select statements

```
const auto a = select(...)...;
const auto b = select(...)...;
const auto c = select(...)...;
```

## Basic use

```C++
for (const auto& row : db(a.union_all(b))) {
    // The result row is determined by the left-hand select.
    // The result row of the right-hand select must be
    // compatible to the left-hand side
    //   - the number of selected expressions must be the same
    //   - the names of selected expressions must be identical in pairs
    //   - the data types of the selected expressions must be
    //     identical in pairs or `std::is_same_v<L, std::optional<R>>`
}

// Union can be chained.
for (const auto& row : db(a.union_all(b).union_all(c))) {
    // The constraints are equivalent to the above, with `a.union_all(b)`
    // representing the left-hand side.
}
```

## Special clauses

`order_by`, `limit`, `offset`, and `for_update` are not allowed as clauses in the
arguments of `union_all` and `union_distinct`.

The former three are available for the union statement, though. For instance

```C++
// order, limit, and offset for the union of `a` and `b`.
for (const auto& row : db(a.union_all(b).order_by(foo.id).limit(7).offset(42))) {
    // ...
}
```

If you want to use `order_by`, `limit`, or `offset` with the union arguments, you
need to wrap them in sub-queries.

```C++
const auto a_o = a.order_by(foo.id).limit(7).offset(42).as(sqlpp::alias::left);

for (const auto& row : db(select(all_of(a_o)).from(a_o).union_all(b))) {
    // ...
}
```

[**\< Index**](/docs/README.md)
