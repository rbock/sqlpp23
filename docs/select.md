[**\< Index**](/docs/README.md)

# Select

Lets assume we have a table representing

```SQL
CREATE TABLE foo (
    id bigint NOT NULL,
    name varchar(50),
    hasFun bool
);
```

(This is SQL for brevity, not C++, see [here]((/docs/ables.md) for details on how to
define types representing the tables and columns you want to work with)

Lets also assume we have an object `db` representing a connection to your
[database](/docs/connection.md).

## A basic example

This shows how you can select some data from table and iterate over the results:

```C++
for (const auto& row : db(select(foo.id, foo.name, foo.hasFun)
                            .from(foo)
                            .where(foo.id > 17 and foo.name.like("%bar%"))))
{
    // `row` objects have data members representing the selected fields.
    // fields that can be NULL have `std::optional` types.

    // row.id is an `int64_t`.
    // row.name is a `std::optional<std::string_view>`.
    // row.hasFun is a `std::optional<bool>`.
}
```

So, what's happening here? Lets ignore the gory details for a moment. Well,
there is a select statement.

```C++
select(foo.id, foo.name, foo.hasFun)
  .from(foo)
  .where(foo.id > 17 and foo.name.like("%bar%"))
```

It selects three columns `id`, `name` and `hasFun` from table `foo` for rows
which match the criteria given in the where condition. That's about as close to
_real_ SQL as it can get...

The select expression is fed into the call operator of the connection object
`db`. This method sends the select statement to the database and returns an
object representing the results. In the case of select statements, the result
object represents zero or more rows.

One way of accessing the rows is to iterate over them in a range-based for loop.

```C++
for (const auto& row : ...)
```

`row` is an object that represents a single result row. You really want to use
`auto` here, because you don't want to write down the actual type. Trust me. But
the wonderful thing about the `row` object is that it has appropriately named
and typed members representing the columns you selected.

## A std::ranges::views example

You process results using ranges::views, too, e.g.

```c++
namespace {
struct my_data {
  int64_t id;
  std::string textNnD;
};
auto row_to_data = [](const auto& row) -> my_data {
  return {row.id, std::string{row.textNnD}};
};
}  // namespace

// ...

void test_views() {
  auto result = db(select(foo.id, foo.textNnD).from(foo));
  const auto vec = result | std::ranges::views::transform(row_to_data) |
                   std::ranges::to<std::vector>();
}
```

## Clauses

### Select

The `select` method takes zero or more named expression arguments.

Named expressions are expressions with a name. No surprise there. But what kind
of expressions have a name? Table columns, for instance. In our example, that
would be `foo.id`, `foo.name` and `foo.hasFun`.

Other expressions, like function calls or arithmetic operations, for instance, do
not have a name per se. But you can give them a name using the
`as(name_provider)` method. The easiest way is to use a named expression as
`name_provider`, e.g.

```C++
const auto unnamed_expression = (foo.id + 17) * 4;
for (const auto& row : db(select(
                unnamed_expression.as(foo.id) // This column is now called `id`.
                ).from(tab).where(foo.id > 42)))
{
   // `row.id` represents the selected expression of type `int64_t`.
}
```

Another option is to define an alias like this:

```C++
// In namespace scope
SQLPP_ALIAS_PROVIDER(total);

[...]

// In a function
for (const auto& row : db(select(sum(id).as(total)).as(foo.id).from(tab).where(true))
{
   std::cout << row.total << std::endl;
}

```

Using aliases also comes in handy when you join tables and have several columns
of the same name, e.g.

```C++
select(foo.id, bar.id);
```

This will result in compile error when accessing `row.id`. One of the columns
needs an alias.

```C++
SQLPP_ALIAS_PROVIDER(barId);

[...]

select(foo.id, bar.id.as(barId));
```

### Select all columns

Statements like `SELECT * from foo` is used pretty often in SQL. sqlpp23 offers
something similar:

```C++
select(all_of(foo)).from(foo).where(condition);
```

### Aggregates

`select` will not let you mix aggregates (see [`group_by`](#group-by) and
[aggregate functions](/docs/aggregate_functions.md)) and non-aggregates, e.g.

````c++
// This will fail to compile as  it mixes aggregate and non-aggregate values.
db(select(
     foo.id,      // non-aggregate
     max(foo.id)) // aggregate
    .from(foo));

If `group_by` is specified in the select, then all columns have to be aggregate expressions.

Otherwise, either

- all selected columns are aggregate expressions (either an aggregate function or a group by column), or
- no selected column is an aggregate expression

### Select with dynamic columns

Columns can be selected conditionally using [`dynamic`](/docs/dynamic.md):

```c++
select(
    foo.id,                          // statically selected
    dynamic(maybe1, foo.textN),       // dynamically selected, if maybe1 == true
    dynamic(maybe2, bar.id.as(barId)) // dynamically selected, if maybe2 == true
    ).from(foo.cross_join(bar)).where(condition);
````

Dynamically selected columns are represented as `std::optional` in the result
row. In case the condition (e.g. `maybe1`) is false:

- the column serializes to `NULL as <name>`
- the field in the result row will be `std::nullopt`

### Constants

`select` requires arguments to have an sqlpp data type and a name. Thus, the following would fail to compile:

```c++
select(7); // 7 has no name
```

The [`value`](/docs/functions.md) function can be used to wrap constant values and give them names:

```c++
for (const auto& row : select(value(7).as(sqlpp::alias::a))) {
    assert(row.a == 7);
}
```

### Alternative syntax for selecting columns

All examples above called the `select()` function with one or more arguments,
but `select()` can also be called with no arguments. In that case, the selected
columns have to be added afterwards

```C++
sqlpp::select().columns(foo.id, foo.name);
```

### Select flags

The following flags are currently supported:

- sqlpp::all
- sqlpp::distinct

Flags can simply be added to the select call before the columns, e.g.

```C++
sqlpp::select(sqlpp::all, foo.id, foo.name);
```

Flags can be added conditionally using [`dynamic`](/docs/dynamic.md):

### From

The `from` method expects one argument. This can be a

- table
- table with an alias
- sub-select with an alias
- join
- CTE

See [tables](/docs/tables.md) for more details.

`from` can also be called with a [`dynamic`](/docs/dynamic.md) argument. In case the
dynamic condition is false, no `FROM` will be included in the serialized
statement.

```c++
select(
    sqlpp::value(7).as(something),
    dynamic(maybe, foo.id))        // foo.id is selected if `maybe == true`
  .from(dynamic(maybe, foo));      // `FROM` clause only present if `maybe == true`
```

References to the table(s) of a dynamic `from` arg must also be dynamic.

### Where

The where condition can be set via the `where` method, which takes a boolean
expression argument, for instance:

```C++
select(all_of(foo)).from(foo).where(foo.id != 17 and foo.name.like("%cake"));
```

`where` can be called with a [`dynamic`](/docs/dynamic.md) argument. In case the
dynamic condition is false, no `WHERE` will be included in the serialized
statement.

```c++
select(foo.id).from(foo)
    .where(dynamic(maybe, foo.id > 17));
```

### Group by

The `group_by` clause takes one or more expression arguments, for instance:

```C++
select(all_of(foo)).from(foo).group_by(foo.name, foo.dep);
```

It can be used to control data ranges for
[`aggregate functions`](/docs/aggregate_functions.md).

`group_by` arguments can be [`dynamic`](/docs/dynamic.md). `dynamic` arguments will
not be serialized if their conditions are false. If all arguments are dynamic
and all conditions are false, the `GROUP BY` clause will not be serialized at
all.

Note that dynamic `group_by` columns can only be used dynamically as aggregate
values `select` or `having`.

### Having

The having condition can be set via the `having` method, just like the `where`
method.

`having` can also be called with a [`dynamic`](/docs/dynamic.md) argument. In case the
dynamic condition is false, no `HAVING` will be included in the serialized
statement.

### Order by

The `order_by` method takes one of more order expression, which are normal
expression adorned with `.asc()` or `.desc()`, e.g.

```C++
select(all_of(foo)).from(foo).order_by(foo.name.asc());
```

`order_by` arguments can be [`dynamic`](/docs/dynamic.md). `dynamic` arguments will
not be serialized if their conditions are false. If all arguments are dynamic
and all conditions are false, the `ORDER BY` clause will not be serialized at
all.

### Limit and offset

The methods `limit` and `offset` take a `size_t` argument, for instance:

```C++
select(all_of(foo)).from(foo).unconditionally().limit(10u).offset(20u);
```

`limit` and `offset` can also be called with [`dynamic`](/docs/dynamic.md) arguments.
In case the dynamic condition is false, the `LIMIT` or `OFFSET` clause will not
be included in the serialized statement.

### For update

The `for_update` method modifies the query with a simplified "FOR UPDATE" clause
without columns.

```C++
select(all_of(foo)).from(foo).where(foo.id != 17).for_update();
```

## Running the statement

OK, so now we know how to create a select statement. But the statement does not
really do anything unless we hand it over to the database:

```C++
db(select(all_of(foo)).from(foo).where(true));
```

This call returns a result object of a pretty complex type. Thus, you would
normally want to use `auto`:

```C++
auto result = db(select(all_of(foo)).from(foo).unconditionally());
```

## Accessing the results

The `result` object created by executing a `select` query is a container of
result rows.

### Range-based for loops

Not surprisingly, you can iterate over the rows using a range-based for-loop
like this:

```C++
for (const auto& row : db(select(all_of(foo)).from(foo)).unconditionally())
{
   std::cerr << row.id << std::endl;
   std::cerr << row.name << std::endl;
}
```

Lovely, isn't it? The row objects have types specifically tailored for the
select query you wrote. You can access their member by name, and these members
have the expected type.

### Function-based access

If for some reason, you don't want to use range-based for-loops, you can use
`front()` and `pop_front()` on the result, like this:

```C++
while(!result.empty())
{
   const auto& row = result.front();
   std::cerr << row.id << std::endl;
   std::cerr << row.name << std::endl;
   result.pop_front();
}
```

[**\< Index**](/docs/README.md)
