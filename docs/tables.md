[**\< Index**](/docs/README.md)

# Tables

In order to build meaningful SQL statements with sqlpp23, you need to represent
tables and their columns as structs that can be understood by the library.

The default way to do so is by using a code generator to translate DDL to C++. A
code generator covering a lot of use cases can be found
[here](https://github.com/rbock/sqlpp23/blob/main/scripts/ddl2cpp).

If you look at the output, you will see why a generator is helpful. Here is a
[sample](https://github.com/rbock/sqlpp23/blob/main/tests/include/sqlpp23/tests/core/tables.h).

## Raw tables

The generated code contains classes representing database tables. Objects can be
default-constructed, e.g.

```c++
constexpr auto foo = test::TabFoo{};
```

Columns of the table are represented as data members of the table objects.

Tables and columns can be used in sqlpp23 statements very similar to SQL tables
and columns can be used in SQL statements, e.g.

```C++
db(insert_into(foo).set(foo.intN = 29, foo.textN = std::nullopt));
for (const auto& row : db(select(all_of(foo)).from(foo).where(foo.id == 17))) {
  // ...
}
```

## Common tables expressions (CTEs)

Non-recursive CTEs are constructed like this:

```c++
const auto x =
    sqlpp::cte(sqlpp::alias::x)      // Call `sqlpp::cte` with a name
      .as(select(foo.id).from(foo)); // Give is it a meaning via as operator.
                                     // The argument has to provide a result
                                     // like a select statement.
```

Recursive CTEs are constructed in two steps:

```c++
// First, you construct the base, e.g.
const auto x_base =
    sqlpp::cte(sqlpp::alias::x).as(select(sqlpp::value(0).as(sqlpp::alias::a)));

// Then, you union with the recursion statement, e.g.
const auto x = x_base.union_all(select((x_base.a + 1).as(sqlpp::alias::a))
                                    .from(x_base)
                                    .where(x_base.a < 10));
```

CTEs are made available to a statement using the [`with`](/docs/with.md) clause.

## Joins

You can join two tables or CTEs like this:

```C++
foo.join(bar).on(foo.id == bar.foo);
```

If you want to join more tables, you can chain joins.

```C++
foo.join(bar).on(foo.id == bar.foo).left_outer_join(baz).on(bar.id == baz.ref);
```

The following join types are supported:

- `join`,
- `inner_join` (this is an alias of join),
- `left_outer_join`,
- `right_outer_join`,
- `full_outer_join`,
- `cross_join` (this is the only join that does not require/allow an `on`
  condition).

## Aliased tables

Both tables and CTEs can be aliased using the `.as()` operator. This can be
useful for self-joins, for instance.

```C++
// Outside of functions
SQLPP_ALIAS_PROVIDER(left);
SQLPP_ALIAS_PROVIDER(right);
SQLPP_ALIAS_PROVIDER(r_id);
[...]

// Inside a function
auto l = foo.as(left);
auto r = foo.as(right);
for (const auto& row : db(select(l.id, r.id.as(r_id))
                             .from(l.join(r).on(l.x == r.y)))) {
  // ...
}
```

Aliased tables might also be used to increase the readability of generated SQL
code, for instance if you have very long table names.

[**\< Index**](/docs/README.md)
