[**\< Index**](/docs/README.md)

# Names

C++ and SQL use names to refer to entities. sqlpp26 automatically translates
between names in C++ and names in SQL for columns and tables, e.g. when you
select columns from a table, the resulting SQL expression contains the correct
names and the members of result rows also refer to the correct names.

```c++
for (const auto& row : db(select(
    foo.id, foo.name, foo.language).from(foo))) {
    // use row.id, row.name, row.language
```

This works because tables and their columns are specifically created such that
they "know" their SQL name.

The C++ name of entities does not matter, though.

```c++
constexpr auto foo = TabFoo{};  // represented as tab_foo in SQL
constexpr auto bar = foo; // also represented as tab_foo in SQL
```

Also, other expressions, like function calls or arithmetic operations, for instance, do
not have a name per se.

If you want to give something an SQL name or if you want to change its SQL name
you can attach that name tag to a table, column, or expression via the `.as()`
function.

## `.as()`

Tables, columns, and expressions in sqlpp26 expose the `.as()` member function.
It takes name template argument and renames the in SQL using the `AS` operator.

```c++
auto foo = TabFoo{};        // a table called tab_foo in SQL
auto id = foo.id;           // a column called id in SQL
auto x = (foo.id + 17) * 4; // an sqlpp expression with no SQL name
auto seven = 7;             // a value with no SQL name

// re-naming a table, e.g. for a self-join
auto left = foo.as<"left">();     // tab_foo AS left
auto right = foo.as<"right">();   // tab_foo AS right

// re-naming a column
id.as<"a">();     // tab_foo.id AS a
left.id.as<"my_fancy_name">();  // left.id AS my_fancy_name

// naming an sqlpp expression
x.as<"id">();               // ((tab_foo.id + 17) * 4) AS id
max(foo.id).as<"max_id">();     // MAX(tab_foo.id) AS max_id

// naming a value
sqlpp::value(seven).as<"s">(); // 7 AS s
```

# Select example
```c++
for (const auto& row :
     db(select(max(foo.id).as<"max_id">(),
               sqlpp::value(seven).as<"seven">())
        .from(foo))) {
    // use row.max_id, row.seven
```

[**\< Index**](/docs/README.md)
