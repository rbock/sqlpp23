[**\< Index**](/docs/README.md)

# From static to dynamic statements

## Fully static

The most straight forward use of sqlpp23 is by constructing and running static
statements, e.g.

```C++
for (const auto& row : db(select(foo.id, foo.name, foo.hasFun)
                            .from(foo)
                            .where(foo.id > 17)))
{
    // Do something with
    // - row.id
    // - row.name
    // - row.hasFun
}
```

In this example, selected columns are fixed (`foo.id`, `foo.name`,
`foo.hasFun`), they belong to exactly one table (`foo`), and the condition
definitely compares the id with a number (`17`).

## Using variables

Values like `17` can be replaced by variables to provide a bit of flexibility:

```C++
int64_t min_id = some_value;
// ...
for (const auto& row : db(select(foo.id, foo.name, foo.hasFun)
                            .from(foo)
                            .where(foo.id > min_id)))
{
    // ...
}
```

## Using prepared statements with parameters

Prepared statements with parameters provide similar flexibility as statements
with variables but at a later stage (after preparing the statement in the
database backend).

```C++
auto prepared_select = db.prepare(select(foo.id, foo.name, foo.hasFun)
                                    .from(foo)
                                    .where(foo.id > parameter(foo.id)));
prepared_select.id = 17;
for (const auto& row : db(prepared_select))
{
    // ...
}

prepared_select.id = 18;
for (const auto& row : db(prepared_select))
{
    // ...
}
```

## `dynamic`

What if you wanted to do some modifications at runtime? Maybe there is a `blob`
that we might want to read under some condition? We can use `dynamic` components
for this. Let's take a look at an example:

```
bool maybe = some_condition;
// ...
for (const auto& row : db(select(foo.id,
                                 foo.name,
                                 foo.hasFun,
                                 dynamic(maybe, bar.blobX))
                            .from(foo.join(dynamic(maybe, bar))
                                     .on(foo.barId == bar.id))
                            .where(foo.id > 17 and
                                   dynamic(maybe, bar.blob.is_not_null())))) {
  // In addition to id, name and hasFun, row also has a member called blobX.
  // If `maybe` is false, blobX is always `nullopt`.
}
```

Depending on the value of `maybe` the structure of the statement changes at
runtime:

- If `maybe == true`, the statement will select an additional column
  (`bar.blobX`), join `foo` with `bar` on the given condition and add another
  condition to the `where` clause.
- If `maybe == false`, the statement will almost be the same as the static
  version above. The only difference is that the result rows will contain a
  column called `blobX` of type `blob` which will contain `nullopt`.

> \[!IMPORTANT\] sqlpp23's ability to validate statements is reduced if
> `dynamic` is used.

- It can check that static components do not depend on dynamic components. In
  the example above, it you could not select `bar.blobX` without wrapping it
  into a `dynamic` call because `bar` is joined dynamically, only.
- If you are using multiple `dynamic` components, the library *cannot* check if
  the conditions match. For instance, you could join on one condition and add a
  column with another. This might end up in incorrect statements.

[**\< Index**](/docs/README.md)
