[**^ Overview**](/README.md)

# Introduction

Let's see:

- You know C++?
- You know some SQL?
- You want to use SQL in your C++ program?
- You think C++ and SQL should play well together?
- You know which tables you want to use in a database?
- You can cope with a few template error messages in case something is wrong?

You have come to the right place!

sqlpp23 offers you to code SQL in C++ almost naturally. You can use tables,
columns and functions. Everything has strong types which allow the compiler to
help you a lot. At compile time, it will tell about most of those pesky
oversight errors you can might make (typos, comparing apples with oranges,
forgetting tables in a select statement, etc). And it does not stop at query
construction. Results can be iterated as ranges, and rows have strongly typed,
aptly named data members, so that you can browse through results in a type-safe
manner.

And of course, code completion in your IDE will/should be able to help writing
correct statements even faster.

The following pages will tell you how to use it:

- **Basics**
  - [Setup](/docs/setup.md)
  - [Code generation](/docs/ddl2cpp.md)
- **Statements**
  - [Database connectors](connectors.md)
  - [Debug logging](logging.md)
  - [Select](/docs/select.md), see also [`with`](/docs/with.md)
  - [Union](/docs/union.md)
  - [Insert](/docs/insert.md)
  - [Update](/docs/update.md)
  - [Delete](/docs/delete.md)
  - [NULL](/docs/null.md)
  - [Dynamic statements](/docs/dynamic.md)
  - [Custom statements](/docs/custom_statements.md)
  - [Exceptions](exception.md)
- **Building Blocks**
  - [Tables, joins, and CTEs](/docs/tables.md)
  - [Functions](/docs/functions.md)
  - [Operators](/docs/operators.md)
  - [Sub-selects](/docs/sub_select.md)
- **Invoking Statements**
  - [Statement execution](/docs/statement_execution.md)
  - [Transaction](/docs/transaction.md)
- **Advanced Topics**
  - [C++ Modules](/docs/modules.md)
  - [Thread safety](/docs/thread_safety.md)
  - [Connection pools](/docs/connection_pool.md)
  - [Recipes](/docs/recipes.md)

If you are coming from [sqlpp11](https://github.com/rbock/sqlpp11), you might be
interested in the [differences](/docs/differences_to_sqlpp11.md).

[**^ Overview**](/README.md)
