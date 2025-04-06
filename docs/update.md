[**\< Index**](README.md)

# Update

You can update rows of a table using an `update` statement:

```C++
db(update(tab).
   .set(tab.gamma = false, tab.textN = std::nullopt)
   .where(tab.id = 17);
```

## `update`

The `update` function takes a single [raw table](tables.md) as its argument.

## `set`

The `set` function has to be called with one or more assigments as its
arguments. The left sides of these assignments have to be columns of the table
mentioned above.

`set` arguments can be [`dynamic`](dynamic.md). Note that if *all* arguments are dynamic with
`false` conditions, the statement becomes invalid.

## `where`

The `where` clause specifies which rows should be affected.

`where` can be called with a [`dynamic`](dynamic.md) argument. In case the
dynamic condition is false, no `WHERE` will be included in the serialized
statement.

[**\< Index**](README.md)
