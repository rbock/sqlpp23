[**\< Recipes**](/docs/recipes.md)

# Working with enums

## What support does sqlpp23 provide for enums?

sqlpp23 does not have built-in support for handling enum data types, so your options for using enums can divides into two broad categories:

  - Explicitly convert your enum to/from a standard type known by the library, each time when you perform a database write or read. In most cases it means that you:
    - Use an integer database column to store your enum values.
    - Cast your C++ enum to an integer before each write.
    - Cast the read integer to a C++ enum after each read.

    This approach works well for small programs with just a few database reads/writes.
  - Tell the library to automatically convert your C++ enum to/from a standard data type that the library already knows how to handle. This can be done by using library's support for [custom types](/docs/recipes/custom_types.md) and then adding a bit of boilerplate code to tell the library how to do the conversion on the fly. This approach works well for bigger programs with multiple database reads/writes.

This document focuses on the second approach and shows step by step how to add the boilerplate code.

## Which C++ enum types can be serialized?

Both `enum` and `enum class` types can be serialized, but check the next section for details and caveats.

## What database column type should I choose for my C++ enum?

There are three questions that you need to have the answer to, before you can choose the best database column type for your C++ enum:

### Does your database support any vendor-specific enum types?

If your database has built-in support for enum types, then you can choose between enum and integer types for your column. If your database does NOT have enum types, then your only option is to use an integer-typed column.

At the time of this writing, [PostgreSQL](https://www.postgresql.org/docs/current/datatype-enum.html) and [MySQL](https://dev.mysql.com/doc/refman/9.7/en/enum.html) have built-in support for enums, while SQLite3 does not. You probably can emulate enums in SQLite3 by using VARCHAR columns and storing the enum value names in those VARCHAR columns, but it might not be worth the trouble and you most likely would be better off using an integer column in SQLite3.

### Are you going to store non-enumerated values in the database?

A C++ enum is not limited to the enumerated values specified in its definition. An enum can contain any integer value that fits into the enum's underlying integer type. If your C++ enum is assigned non-enumerated values during the course of the program execution, then you most likely want to use an integer data column, because database vendor-specific enum extensions typically limit the enum fields to a fixed set of string values.

### Is the serialization/deserializations of enums performance-critical?

Working with vendor-specific enum fields is usually a bit slower than working with generic integer fields, because serialization to enum fields usually means that you have to check the enum value and generate the corresponding string value, and deserialization from an enum field means that you have to make a few string comparisons until you find the matching enum value. On the other hand, serializing to and deserializing from an integer column requires just a static_cast between an enum and an integer, an operation that takes zero CPU cycles because it is done at compile time.

So if the enum serialization/deserialization could become a performance bottleneck for your application, you should probably use an integer database column.

### Conclusion

To summarize the above points, you _could_ use a vendor-specific enum field type if all the following conditions are true:

  - Your database has vendor-specific support for enum fields.
  - Your C++ enums are not assigned non-enumerated values.
  - The serialization/deserialization of this enum is not performance-critical for your application.

If any of the above conditions are not met, then you are _probably_ better off just using an integer database column.

## What is the actual boilerplate code required to enable serialization/deserialization of an enum type?

Since enum serialization/deserialization relies on sqlpp23's support for [custom types](/docs/recipes/custom_types.md) to perform the actual work, this means that you basically have to add the same template specializations and functions that you add a C++ custom type to sqlpp23. Let's say that we have an enum with the following definition:
```
enum class animal { bird, cat, dog, fish };
```
We define this enum in our program's namespace, and we want to enable its serialization/deserialization to/from a nullable integer column. To do that we add the following pieces of code:

### Helper metafunctions that check if a variable is of our (possibly optional) enum type

```
template <typename T>
struct is_animal
    : public std::is_same<sqlpp::remove_optional_t<sqlpp::data_type_of_t<T>>,
                          animal> {};
template <typename T>
static constexpr bool is_animal_v = is_animal<T>::value;
```

These are helper metafunctions that are not required by the library per se, but we use them to factor out some common code.

### The actual serialization function

```
template <typename Context>
auto to_sql_string(Context& context, const animal& value) {
  return sqlpp::to_sql_string(context, static_cast<int64_t>(value));
}
```

This is the function that actually does the serialization of our enum. It should be defined in the namespace where our enum is defined, so that the library will be able to find it through Argument-Dependent Lookup (ADL).

### The function that extracts the enum from a database results

```
template <typename Result>
void read_field(const Result& result, size_t field_index, animal& value) {
  int64_t iv{};
  read_field(result, field_index, iv);
  value = static_cast<animal>(iv);
}
```

This function will be called when the library reads a database row and comes across a field that holds our serialized enum. It should be defined in the namespace where our enum is defined, so that the library will be able to find it through Argument-Dependent Lookup (ADL).

### Optionally enable use of our enum as a parameter in prepared statements

```
template <typename Statement>
void bind_parameter(Statement& stmt, size_t index, const animal& value) {
  bind_parameter(stmt, index, static_cast<int64_t>(value));
}
```
An optional function that serializes the enum value when it is used as a parameter in prepared statements. If defined, it should reside in the namespace where our enum is defined, so that the library will be able to find it through Argument-Dependent Lookup (ADL).

### Enable the library to calculate the data type of any expression that uses our enum

```
template <>
struct data_type_of<animal> {
  using type = animal;
};
```

A metafunction that let's the library calculate the data type of any expression that uses our enum. Without it any such expression would cause an error at compile time. It has to be defined in the `sqlpp` namespace.

### Enable reading and deserialization of the enum from the database

```
template <>
struct result_data_type_of<animal> {
  using type = animal;
};
```

A metafunction that tells the library that our enum can be read from the database (usually by SELECT) and then deserialized to a C++ enum. It has to be defined in the `sqlpp` namespace.

### Enable enum assignment during INSERT queries

```
template <typename L, typename R>
  requires(is_animal_v<L> && is_animal_v<R>)
struct values_are_assignable<L, R> : public std::true_type {};
```

This metaflag tells the library that our enum values can be assigned in `INSERT` or `UPDATE` statements. It has to be defined in the `sqlpp` namespace.

### Optionally enable comparison of our enum values

```
template <typename L, typename R>
  requires(is_animal_v<L> && is_animal_v<R>)
struct values_are_comparable<L, R> : public std::true_type {};
```

This metaflag tells the library that our enum values can be compared. Only needed if you want to use the enum in ORDER BY clauses or compare it using ==, <, etc. If defined, it has to reside in the `sqlpp` namespace.

That's it! After defining these metafunctions and metaflags, the library will be able to serialize/deserialize the enum.

## Sample code

See [this sample project](/tests/postgresql/recipes/enum/), which defines two enums. The first enum type, `animal` is serialized to a nullable integer column. The second enum type `shape` is serialized to a nullable enum column, which uses a PostgreSQL-specific extension to define the enum column. Then the project writes and then reads back several rows that hold enum values.

[**\< Recipes**](/docs/recipes.md)
