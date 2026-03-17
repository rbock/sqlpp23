[**\< Index**](/docs/README.md)

# Names

C++ and SQL use names to refer to entities. sqlpp23 automatically translates
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

If you want to give something an SQL name or if you want to change its SQL name,
you can either

- use the name of something that already has a name
- prepare a name tag

and attach that name tag to a table, column, or expression via the `.as()`
function.

## Name tags

Name tags are created using the `SQLPP_CREATE_NAME_TAG` macro. It has to be used
outside of functions (typically in an unnamed namespace), e.g.

```
// Outside of function scope.
SQLPP_CREATE_NAME_TAG(my_fancy_name);
SQLPP_CREATE_NAME_TAG(max_id);
```

sqlpp23 also provides a couple of convenience name tags in the `sqlpp::alias`
namespace. These are

```
sqlpp::alias::a;
// ...
sqlpp::alias::z;
sqlpp::alias::left;
sqlpp::alias::right;
```

## `.as()`

Tables, columns, and expressions in sqlpp23 expose the `.as()` member function.
It takes name tag (see above) or an expression that already has a name as an
argument and renames the in SQL using the `AS` operator.

```c++
auto foo = TabFoo{};        // a table called tab_foo in SQL
auto id = foo.id;           // a column called id in SQL
auto x = (foo.id + 17) * 4; // an sqlpp expression with no SQL name
auto seven = 7;             // a value with no SQL name

// re-naming a table, e.g. for a self-join
auto left = foo.as(sqlpp::alias::left);     // tab_foo AS left
auto right = foo.as(sqlpp::alias::right);   // tab_foo AS right

// re-naming a column
id.as(sqlpp::alias::a);     // tab_foo.id AS a
left.id.as(my_fancy_name);  // left.id AS my_fancy_name

// naming an sqlpp expression
x.as(foo.id);               // ((tab_foo.id + 17) * 4) AS id
max(foo.id).as(max_id);     // MAX(tab_foo.id) AS max_id

// naming a value
sqlpp::value(seven).as(sqlpp::alias::s); // 7 AS s
```

## C++26 reflection (experimental)

C++26 offers [reflection](https://wg21.link/P2996). One aspect of that is an improved
meta-programming access to names.

When used with C++26 and reflection support, the library offers another overload of the
`.as` function.

```c++
// with reflection, you can use a string literal as template argument to name or
// rename entities.
foo.as<"left">();             // tab_foo AS left
max(left.id).as<"max_id">();  // MAX(left.id) AS max_id

// alternatively you can use the user-defined literal "_alias" instead of template arguments.
using namespace sqlpp::literals;  // using namespace required in scope

foo.as("left"_alias);             // tab_foo AS left
max(left.id).as("max_id"_alias);  // MAX(left.id) AS max_id
```

**Experimental**
As of this writing (2026-02), this is known to compile with g++-16.0.1 with `-freflection`.

[**\< Index**](/docs/README.md)
