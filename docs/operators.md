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

*   At least one `.then()` clause or the `.else_()` clause must be called with an argument different from `std::nullopt`.
*   The data type of the entire CASE expression is determined by the data type of the *first* non-null argument of a `.then()` clause (or the `else_()` clause if the arguments of all `.then() clauses are `std::nullopt`).
*   All subsequent `.then()` arguments, as well as `.else_()` argument, must have the same data type (nullability may differ) as this first non-null argument.
*   The `case` expression can be NULL if any of the chosen `.then()` or `.else_()` expressions can be NULL.

**Examples:**

Let's assume we have a table `foo` with columns `id (INTEGER)`, `name (TEXT)`, and `category (INTEGER)`.

**1. Simple CASE expression:**
Map `category` id to a string representation.

```c++
// Assuming foo.category and relevant string values/columns
const auto category_name = ::sqlpp::case_when(foo.category == 1).then("Category A")
                               .when(foo.category == 2).then("Category B")
                               .else_("Unknown Category");

// Example usage in a SELECT statement:
// for (const auto& row : db(select(foo.name, category_name.as(sqlpp::alias::a)).from(foo)...)) {
//   std::cout << row.name << ": " << row.a;
// }
```

**2. Using `std::nullopt`**
If the first `then` argument is not NULL, its type determines the overall non-optional type.
If `std::nullopt` is used in a subsequent `then` or in `else_`, the entire CASE expression becomes nullable.

```c++
const auto conditional_name = ::sqlpp::case_when(foo.category == 1).then(foo.name)
                                  .when(foo.category == 3).then(std::nullopt) // Makes expression potentially NULL
                                  .else_(sqlpp::value("Default Name"));
```

If the first `then` argument is NULL, then the type of the expression is determined by the first non-NULL
`then` or `else_` argument. The entire CASE expression is nullable.

```c++
// `then` argument is NULL. `else_` determines the data type of the expression
const auto another_value = ::sqlpp::case_when(foo.id > 100).then(std::nullopt)
                              .else_(foo.id);
```

[**\< Index**](/docs/README.md)
