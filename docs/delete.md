[**< Index**](README.md)

# Delete

Deleting rows from tables is straight forward:

```C++
db(delete_from(tab).where(tab.alpha == 35));
```

## Truncate

If you want to delete all rows from a table, the truncate function might be the
faster option.

```c++
db(truncate(tab));
```

## The `using` clause

Some backends support `where` conditions that use additional tables. These are
supplied to the statement via the `using` clause.

```C++
db(delete_from(foo).using_(bar).where(
    foo.bar_id == bar.id
    and bar.username == "johndoe"
    ));
```

[**< Index**](README.md)
