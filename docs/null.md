[**< Index**](/docs/README.md)

# `NULL`

Columns and values in SQL can be `NULL`. sqlpp23 represents them as
`std::optional`. In particular, `NULL` is represented as `std::nullopt`.

Reading fields form a result row of a `select` or assigning values in an
`insert` or `update` is straight forward this way. C++ and SQL behave in the
same way.

It is a bit different with comparisons. SQL `NULL` is a strange beast from a C++
point of view. It can be compared to anything but that comparison never returns
`true`. It also never returns `false`, it returns `NULL`. Even when you compare
`NULL` to itself, the result is `NULL`.

```SQL
NULL != NULL    -> NULL
NULL = NULL     -> NULL
```

sqlpp23 does not change that behavior as it is totally valid SQL. The library
therefore also mimics a few more operators that help dealing with `NULL`:

## `IS NULL` and `IS NOT NULL`

This checks if a value is or isn't `NULL`.

```c++
// ...
   .where(foo.name.is_null() and (foo.id + bar.offset).is_not_null());
```

## `IS DISTINCT FROM` and `IS NOT DISTINCT FROM`

These operators offer a `NULL`-safe comparison. They will return boolean results
even if either or both operands are `NULL`.

`IS DISTINCT FROM` resembles
`std::optional<T>::operator!=`.\
`IS NOT DISTINCT FROM` resembles
`std::optional<T>::operator==`.

The can be used like this:

```c++
// ...
   .where(foo.id.is_distinct_from(something));
```

## Selecting `NULL`

Columns in an `sqlpp::select` require a name and a data type.
If you want to select `NULL` explicitly, you can do so using the `sqlpp::value` wrapper
function.

```c++
select(sqlpp::value(
            std::optional<int64_t>{} // i.e. std::nullopt, but with an explicit data type
            ).as(sqlpp::alias::a));  // provide a name
```

[**< Index**](/docs/README.md)
