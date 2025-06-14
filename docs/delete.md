[**< Index**](/docs/README.md)

# Delete

Deleting rows from tables is straight forward:

```C++
db(delete_from(tab).where(tab.alpha == 35));
```

## `where`

The `where` clause specifies which rows should be affected.

`where` can be called with a [`dynamic`](/docs/dynamic.md) argument. In case the
dynamic condition is false, no `WHERE` will be included in the serialized
statement.

> [!NOTE]
> Each of the [connectors](/docs/connectors.md) is offering a specialized version of `delete`.

## Truncate

If you want to delete all rows from a table, the truncate function might be the
faster option.

```c++
db(truncate(tab));
```

> [!NOTE]
> sqlite3 does not support `TRUNCATE`. The sqlite3/SQLCipher connector
> therefore serializes this as unconditional `DELETE FROM`.

[**< Index**](/docs/README.md)
