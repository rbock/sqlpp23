[**< Index**](/docs/README.md)

# Sub-Select

## Sub-Select as a value

A sub select with a single column and a single result row can be used as a value, for instance in `where`, e.g.

```c++
for (const auto& row :
     db(select(all_of(foo))
            .from(foo)
            .where(foo.text ==
                   select(bar.text).from(bar).where(bar.id == foo.id)))) {
  const int x = row.id;
  const int b = row.cheese_cake;
}
```

## Sub-Select as a selected value

As seen above, a sub select statement with single column and a single result row can be used as a value. But it does not have a name.
To use it a sub select as a column, you need to wrap in `sqlpp::value` and give the whole thing a name, e.g.

```c++
SQLPP_ALIAS_PROVIDER(cheese_cake); // Declared outside of function
// ...
for (const auto& row :
     db(select(all_of(foo),
               value(select(bar.text).from(bar).where(bar.id == foo.id))
                   .as(cheese_cake))
            .from(foo))) {
  const int x = row.id;
  const int b = row.cheese_cake;
}
```
# Sub-Select as a value set

A sub select with a single column can be used in combination with operators like `exist`, `any`, or `in`, e.g.

```c++
for (const auto& row :
     db(select(all_of(foo))
            .from(foo)
            .where(foo.text.in(
                select(bar.text).from(bar).where(bar.id > foo.id))))) {
  const int x = row.id;
  const int b = row.cheese_cake;
}
```

### Sub-Select as a table

A select can be used as a pseudo table in another select. You just have to give
it a name.

```c++
SQLPP_ALIAS_PROVIDER(sub); // Declared outside of functions
// [...]

auto sub_select = select(all_of(foo)).from(foo).where(foo.id == 42).as(sub);
```

The variable `sub_select` can be used as a table now, e.g.

```c++
for (const auto& row : db(select(sub_select.id).from(sub_select))) {
    register_id(row.id);
}
```

[**< Index**](/docs/README.md)
