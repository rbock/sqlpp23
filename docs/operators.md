[**\< Index**](/docs/README.md)

# Operators

This page describes various SQL operators and expressions available in sqlpp23.

## CASE Operator

The CASE operator allows for conditional expression evaluation, similar to an if/then/else structure in programming languages. sqlpp23 provides a fluent interface to construct CASE expressions.

**Syntax:**

The general flow to build a CASE expression is:

`::sqlpp::case_when(condition1).then(result1)`
`    [.when(condition2).then(result2)]...`
`    .else_(else_result)`

*   It starts with `::sqlpp::case_when(condition)`.
*   Followed by `.then(result_expression)`.
*   Optionally, one or more additional `.when(condition).then(result_expression)` clauses can be chained.
*   It must end with an `.else_(else_result_expression)` clause.

**Return Type and Type Compatibility:**

*   The data type of the entire CASE expression is determined by the data type of the result expression in the *first* `.then()` clause.
*   All subsequent result expressions in further `.then()` clauses, as well as the result expression in the `.else_()` clause, must be comparable with this first result type. The library uses `::sqlpp::values_are_comparable` to enforce this at compile time. If types are not comparable, a static assertion will fail.
*   The CASE expression can be NULL if any of the chosen `THEN` or `ELSE` expressions can be NULL.

**Examples:**

Let's assume we have a table `foo` with columns `id (INTEGER)`, `name (TEXT)`, and `category (INTEGER)`.

**1. Simple CASE expression:**
Map `category` id to a string representation.

```cpp
// Assuming foo.category and relevant string values/columns
const auto category_name = ::sqlpp::case_when(foo.category == 1).then(sqlpp::value("Category A"))
                               .when(foo.category == 2).then(sqlpp::value("Category B"))
                               .else_(sqlpp::value("Unknown Category"));

// Example usage in a SELECT statement:
// for (const auto& row : db(select(foo.name, category_name.as(alias::category_name)).from(foo)...)) {
//   std::cout << row.name << ": " << row.category_name;
// }
```

**2. CASE expression with different result types (demonstrating comparability):**
If `foo.id` is `BIGINT` and `sqlpp::value(0)` defaults to `INTEGER`, they are comparable.

```cpp
const auto derived_value = ::sqlpp::case_when(foo.name.like("%Special%")).then(foo.id)
                               .else_(sqlpp::value(0)); 
```

**3. Using `std::nullopt` for a result:**
The type of the first `then` (e.g., `foo.name` which might be `TEXT`) determines the overall non-optional type.
If `std::nullopt` is used in a subsequent `then` or in `else_`, the entire CASE expression becomes nullable.

```cpp
// foo.name is TEXT, foo.id is BIGINT (for example)
// Let's say first .then() returns TEXT.
const auto conditional_name = ::sqlpp::case_when(foo.category == 1).then(foo.name)
                                  .when(foo.category == 3).then(std::nullopt) // Makes expression potentially NULL
                                  .else_(sqlpp::value("Default Name"));

// If the first .then() itself returned std::nullopt, or a nullable column:
const auto another_value = ::sqlpp::case_when(foo.id > 100).then(std::nullopt)
                              .else_(foo.id); // foo.id must be comparable to NULL here.
                                             // The case expression will be nullable.
```

[**\< Index**](/docs/README.md)
