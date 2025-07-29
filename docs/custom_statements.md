[**\< Index**](/docs/README.md)

# Custom statements

## `verbatim_table`

`verbatim_table` can be used to add tables to statements even if the table is not known at compile time.

This would typically be used in combination with `verbatim` expressions, see below.

## `verbatim`

The `verbatim` function can be used to inject custom strings into statements.

It can be used as custom value expression:

```c++
// custom expression requires the data type as template parameter:
db(update(t).set(t.intN = sqlpp::verbatim<sqlpp::integral>("some expression")));

// Combination with verbatim_table:
for (const auto& row :
     db(select(sqlpp::verbatim<sqlpp::text>("my_table.id").as(sqlpp::alias::a))
            .from(sqlpp::verbatim_table("my_table")))) {
  handle_text(row.a);
}
```

`verbatim` can also be used as a custom clause:

```c++
// components of a statement can be combined into custom statement using
// the `<<` operator.
for (const auto& row :
     db(select(all_of(t)).from(t)
        << sqlpp::verbatim("WINDOW window_name AS (window_definition)")
        << order_by(t.id))) {
  // ...
}
```

## `with_result_type_of`

It might be necessary to tell the library the correct return type of a custom statement.
This can be done by adding the `with_result_type_of` function at the end of the statement.
It won't serialize its arguments, but change the return type of the statement, e.g.

```c++
// See www.sqlite.org/pragma.html#pragma_user_version
// PRAGMA user_version returns an integer
// We can tell the library to treat this like a select of a single column
// with the correct type.
for (const auto& row :
     db(sqlpp::statement_t{} << sqlpp::verbatim("PRAGMA user_version")
                             << with_result_type_of(select(t.id)))) {
  user_version = row.id;
}

// If you really want the statement to contain "SELECT *" instead of every
// column explicitly (maybe you are running into size limits?), this is one
// way to do it.
// Not recommended, though, as it is clearly more prone to errors than the
// canonical way.
for (const auto& row :
   db(sqlpp::statement_t{} << sqlpp::verbatim("SELECT *") << from(t)
                           << where(t.id > 17)
                           << with_result_type_of(select(all_of(t))))) {
(void)row.id;
}
```

## `parameterized_verbatim`

The super power of sqlpp23 is that it can detect incorrect statements at compile time.
`verbatim` can potentially hide mistakes. `parameterized_verbatim` combines the flexibility of
`verbatim` with the type system of sqlpp23. It takes three arguments: a string, an expression, and another string.

```c++
// quickly simulate an unknown function
select(t.id).from(t).where(
    sqlpp::parameterized_verbatim<
        sqlpp::unsigned_integral>("ABS(", t.id - t.offset, ")") > 0);
```

The expression can contain parameters, too:

```c++
  auto abs = db.prepare(select(t.id).from(t).where(
      sqlpp::parameterized_verbatim<sqlpp::unsigned_integral>(
          "ABS(field1 -", sqlpp::parameter(t.id), ")") <=
      sqlpp::parameter(sqlpp::unsigned_integral(), param2)));
  abs.parameters.id = 7;
  abs.parameters.param2 = 7;
```

## `flatten`

To increase the chance of creating correct `verbatim` expressions, it might make sense to
construct an expression and then turn it into a `verbatim` expression via the `flatten` function.

```
// Assuming value == 17, this would be equivalent to
// sqlpp::verbatim<sqlpp::integral>("something + 17")
flatten(sqlpp::verbatim<sqlpp::integral>("something") + value);
```

[**\< Index**](/docs/README.md)
