[**< Index**](/docs/README.md)

# Functions

This page documents the various non-aggregate functions available in sqlpp23, see also [aggregate functions](/docs/aggregate_functions.md).

When selected, functions need to be assigned an alias, i.e.

```c++
// functions calls can be used where values are required.
db(insert_into(tab).set(tab.name = sqlpp::trim(some_name)));

// function calls must be given an alias to be selected.
db(const const auto& row : db(select(upper(tab.name).as(sqlpp::alias::a)))) {
    // do something
}
```

## Text functions

### `concat`

The `concat` function concatenates one or more text expressions.

Arguments can be [`dynamic`](/docs/dynamic.md). Dynamic arguments with no `false` condition are interpreted as `NULL`.

**Example:**

```cpp
// SQL: CONCAT(users.first_name, ' ', users.last_name)
sqlpp23::concat(users.first_name, " ", users.last_name);

// SQL: CONCAT('Username: ', users.first_name)
sqlpp23::concat("Username: ", users.first_name);
```

Note that this is serialized differently for postgresql, using the `||` operator.

### `lower`

The `lower` function converts a string expression to lowercase.

**Example:**

```cpp
// SQL: LOWER(products.name)
sqlpp23::lower(products.name);

// SQL: SELECT LOWER('THIS IS AN UPPERCASE STRING')
sqlpp23::lower("THIS IS AN UPPERCASE STRING");
```

### `trim`

The `trim` function removes leading and trailing whitespace characters from a string.

**Example:**

```cpp
// SQL: TRIM(docs.title)
sqlpp23::trim(docs.title);

// SQL: TRIM('  extra spaces  ')
sqlpp23::trim("  extra spaces  ");
```

## `upper`

The `upper` function converts a string expression to uppercase.

**Example:**

```cpp
// SQL: UPPER(articles.title)
sqlpp23::upper(articles.title);

// SQL: SELECT UPPER('all lower string')
sqlpp23::upper("all lower string");
```

## Date / time functions

### `current_date`

The `current_date` function returns the current date as determined by the SQL database.

**Example:**

```cpp
// SQL: CURRENT_DATE
sqlpp23::current_date();
```

### `current_time`

The `current_time` function returns the current time of day as determined by the SQL database.

**Example:**

```cpp
// SQL: CURRENT_TIME
sqlpp23::current_time();
```

### `current_timestamp`

The `current_timestamp` function returns the current timestamp (date and time) as determined by the SQL database.

**Example:**

```cpp
SQLPP_CREATE_NAME_TAG(current_timestamp_alias);

// SQL: SELECT CURRENT_TIMESTAMP
sqlpp23::current_timestamp();
```

## Miscellaneous

### `coalesce`

The SQL `COALESCE` function returns the first non-null expression among its arguments.

The `coalesce` function takes one or more arguments. All arguments must have compatible data types
(differences in optionality (e.g., `sqlpp::integral` vs `std::optional<sqlpp::integral>`) are permissible).

Arguments can be [`dynamic`](/docs/dynamic.md). Dynamic arguments with no `false` condition are interpreted as `NULL`.

**Example:**

```cpp
// Assuming
// * my_table.int_column (integral)
// * my_table.text_column (text)
// * my_table.another_column (text)

// SQL: COALESCE(my_table.int_column, 42)
sqlpp23::coalesce(my_table.int_column, 42);

// SQL: COALESCE(my_table.text_column, my_table.another_column, 'default_value')
sqlpp23::coalesce(my_table.text_column, my_table.another_column, "default_value");

// Compile error (must not mix different data types)
sqlpp23::coalesce(my_table.int_column, "default_value");
```

### `value`

Most of the time, raw values like `7` or `"seven"` can be used in expressions just like that, e.g.

```c++
select((tab.id + 3).as(something))
    .from(tab)
    .where(tab.id == 7 or tab.name == "seven");
insert_into(tab).set(tab.name = "eight");
```

But if member functions like `.as()` or comparison methods `.in()`, `not_in()`, `is_null()`, `is_not_null()`, `is_distinct_from()`, or `not_is_distinct_from()` are needed, then such raw values need to be wrapped by the `value` function, e.g.

```c++
select(value(7).as(something), tab.id)
    .from(tab)
    .where(value("seven").in(tab.first, tab.last));
```

The other use of the `value` function is to wrap a [sub select](/docs/sub_select.md) for use as a selected column.

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

Beyond that, `value` can be used to wrap raw values and thereby move them into the `sqlpp` namespace. This *might* be useful for function lookup:

```c++
auto get_seven() {
    return 7;
}

auto get_value() {
    return sqlpp::value(7);
}

// ...

sqlpp::dynamic(true, get_seven()); // requires namespace qualification
dynamic(true, get_value()); // namespace determined automatically
```

[**< Index**](/docs/README.md)

