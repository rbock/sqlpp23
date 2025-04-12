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
  - [Setup](setup.md)
  - [Code generation](ddl2cpp.md)
- **Statements**
  - [Connection](connection.md)
  - [Select](select.md), see also [`with`](with.md)
  - [Insert](insert.md)
  - [Update](update.md)
  - [Delete](delete.md)
  - [NULL](null.md)
  - [Static vs. Dynamic](dynamic.md)
- **Building Blocks**
  - [Tables, Joins, and CTEs](tables.md)
  - [Functions](functions.md)
  - [Sub Selects](sub_select.md)
- **Invoking Statements**
  - [Statement execution](statement_execution.md)
  - [Transaction](transaction.md)
- **Advanced Topics**
  - [Thread Safety](thread_safety.md)
  - [Connection Pool](connection_pool.md)

If you are coming from [sqlpp11](https://github.com/rbock/sqlpp11), you might be
interested in the [differences](differences-to-sqlpp11.md).
