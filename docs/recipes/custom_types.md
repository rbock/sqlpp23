[**\< Recipes**](/docs/recipes.md)

# Custom types

sqlpp23 can be extended with user-defined C++ types that the library treats as
first-class column types: they appear in result rows, in `WHERE` and `SET`
expressions, and in prepared statement parameter structs — all with full
compile-time type checking.

This recipe walks through the complete implementation using a `Point` table
whose `x` and `y` columns carry distinct types `XCoord` and `YCoord`. Because
these are different types, the compiler will reject any attempt to assign an
`XCoord` value to a `YCoord` column or compare them with each other — even
though both wrap `int64_t` internally.

---

## The custom types

```cpp
struct XCoord { int64_t value; };
struct YCoord { int64_t value; };
```

The rest of the recipe shows what is needed to make sqlpp23 understand these
types.

---

## 1. Custom type implementation

Every custom type needs six things. The first three are free functions placed
near the type definition; the remaining four are template specialisations in
`namespace sqlpp`.

### Free functions (ADL-discovered, live near the type)

**`to_sql_string`** — serialises the value to SQL text. Delegate to the
underlying primitive:

```cpp
template <typename Context>
auto to_sql_string(Context& context, const XCoord& v) {
  using sqlpp::to_sql_string;
  return to_sql_string(context, v.value);
}
```

**`read_field`** — populates the type from a result row field. The `Result`
template parameter makes this work with every connector (PostgreSQL, MySQL,
SQLite3, …) without any connector-specific code:

```cpp
template <typename Result>
void read_field(const Result& result, size_t index, XCoord& v) {
  read_field(result, index, v.value);
}
```

**`bind_parameter`** — binds the type as a prepared statement parameter. The
`Statement` template parameter works across all connectors for the same reason:

```cpp
template <typename Statement>
void bind_parameter(Statement& stmt, size_t index, const XCoord& v) {
  bind_parameter(stmt, index, v.value);
}
```

### Template specialisations in `namespace sqlpp`

**`data_type_of<T>`** — declares the type's data type tag. Using the type
itself as its own tag (rather than mapping to `sqlpp::integral`) gives each
custom type its own identity, so `XCoord` and `YCoord` columns cannot be mixed:

```cpp
namespace sqlpp {

template <>
struct data_type_of<XCoord> {
  using type = XCoord;   // XCoord is its own tag
};
```

**`result_data_type_of<Tag>`** — declares what type a result row field holds
for this tag. With the self-tagging pattern this is always the type itself:

```cpp
template <>
struct result_data_type_of<XCoord> {
  using type = XCoord;
};
```

**`parameter_value<Tag>`** — declares the type held in a prepared statement
parameter struct. Again, the type itself:

```cpp
template <>
struct parameter_value<XCoord> {
  using type = XCoord;
};
```

**`values_are_assignable<L, R>`** — controls whether `R` can be assigned to an
`L` column (i.e. used in `SET` and `INSERT`). Without this, `.set(t.x = ...)`
will not compile:

```cpp
template <typename L, typename R>
  requires(is_xcoord_v<L> and is_xcoord_v<R>)
struct values_are_assignable<L, R> : public std::true_type {};
```

**`values_are_comparable<L, R>`** — controls whether `L` and `R` can be
compared (i.e. used in `WHERE` with `==`, `<`, `>`, etc.) and sorted (i.e.
used in `ORDER BY` via `.asc()` / `.desc()`):

```cpp
template <typename L, typename R>
  requires(is_xcoord_v<L> and is_xcoord_v<R>)
struct values_are_comparable<L, R> : public std::true_type {};

} // namespace sqlpp
```

### The `is_xcoord` helper

The `requires` clauses above need a predicate that recognises all types whose
data type is `XCoord` — both a bare `XCoord` value and a column whose
`data_type` is `XCoord`. The pattern mirrors sqlpp23's own `is_integral`,
`is_text`, etc.:

```cpp
template <typename T>
struct is_xcoord : public std::is_same<sqlpp::data_type_of_t<T>, XCoord> {};

template <typename T>
inline constexpr bool is_xcoord_v = is_xcoord<T>::value;
```

---

## 2. Table definition

`ddl2cpp` maps SQL type names to built-in sqlpp23 tags (`integral`, `text`,
etc.) and cannot emit a user-defined C++ type as a column's `data_type`.
Columns that carry a custom type must therefore be specified manually in the
column spec. Only the `data_type` line differs from what `ddl2cpp` would
generate for that column:

```cpp
// tables/tab_point.h
#pragma once
#include "coord_types.h"   // XCoord, YCoord and their sqlpp23 traits

namespace myapp {

struct TabPoint_ {
  struct Id {
    SQLPP_CREATE_NAME_TAG_FOR_SQL_AND_CPP(id, id);
    using data_type   = ::sqlpp::integral;
    using has_default = std::true_type;   // SERIAL / auto-increment
  };
  struct X {
    SQLPP_CREATE_NAME_TAG_FOR_SQL_AND_CPP(x, x);
    using data_type   = XCoord;           // custom type as data_type
    using has_default = std::false_type;  // required on INSERT
  };
  struct Y {
    SQLPP_CREATE_NAME_TAG_FOR_SQL_AND_CPP(y, y);
    using data_type   = YCoord;           // custom type as data_type
    using has_default = std::false_type;
  };
  SQLPP_CREATE_NAME_TAG_FOR_SQL_AND_CPP(tab_point, tabPoint);
  template <typename T>
  using _table_columns = sqlpp::table_columns<T, Id, X, Y>;
  using _required_insert_columns = sqlpp::detail::type_set<
      sqlpp::column_t<sqlpp::table_t<TabPoint_>, X>,
      sqlpp::column_t<sqlpp::table_t<TabPoint_>, Y>>;
};
using TabPoint = ::sqlpp::table_t<TabPoint_>;

}  // namespace myapp
```

> Columns with `has_default = std::false_type` appear in
> `_required_insert_columns` and must be provided in every
> `insert_into().set()`.

---

## 3. Example queries

```cpp
#include "tables/tab_point.h"

myapp::TabPoint t;
```

### SELECT

`row.x` and `row.y` are `XCoord` and `YCoord` directly — no casting or
unwrapping at the call site. `ORDER BY` works because `values_are_comparable`
is specialised:

```cpp
for (const auto& row : db(
    select(all_of(t))
    .from(t)
    .where(t.x > XCoord{5})
    .order_by(t.y.asc())))
{
  static_assert(std::is_same_v<decltype(row.x), XCoord>);
  static_assert(std::is_same_v<decltype(row.y), YCoord>);

  use(row.x.value, row.y.value);
}
```

The compiler rejects `t.x > YCoord{5}` because `values_are_comparable<XCoord,
YCoord>` is not specialised.

### INSERT

Both `X` and `Y` are required columns, so both must be provided:

```cpp
db(insert_into(t).set(
    t.x = XCoord{10},
    t.y = YCoord{20}
));
```

`t.x = YCoord{20}` is a compile error: `values_are_assignable<XCoord, YCoord>`
is not specialised.

### UPDATE

```cpp
db(update(t)
    .set(t.x = XCoord{99},
         t.y = YCoord{42})
    .where(t.id == 1));
```

### DELETE

```cpp
db(delete_from(t)
    .where(t.x == XCoord{0} and t.y == YCoord{0}));
```

---

## 4. Prepared statements

Because `parameter_value<XCoord>::type = XCoord`, the parameter struct holds
`XCoord` directly — the same full round-trip as with regular queries.

### Prepared SELECT

```cpp
auto stmt = db.prepare(
    select(all_of(t))
    .from(t)
    .where(t.x > sqlpp::parameter(t.x))
);

stmt.parameters.x = XCoord{5};   // XCoord directly; param name matches column name

for (const auto& row : db(stmt)) {
  XCoord x = row.x;          // XCoord on the read side too
  use(x.value);
}
```

### Prepared INSERT

```cpp
auto stmt = db.prepare(
    insert_into(t).set(
        t.x = sqlpp::parameter(t.x),
        t.y = sqlpp::parameter(t.y)
    )
);

stmt.parameters.x = XCoord{10};
stmt.parameters.y = YCoord{20};
db(stmt);
```

### Prepared UPDATE

```cpp
auto stmt = db.prepare(
    update(t)
    .set(t.x = sqlpp::parameter(t.x))
    .where(t.id == sqlpp::parameter(t.id))
);

stmt.parameters.x  = XCoord{99};
stmt.parameters.id = 1;
db(stmt);
```

### Prepared DELETE

```cpp
auto stmt = db.prepare(
    delete_from(t)
    .where(t.x == sqlpp::parameter(t.x))
);

stmt.parameters.x = XCoord{0};
db(stmt);
```

> `sqlpp::parameter(col)` derives both the data type and the parameter name
> from the column itself, so `stmt.parameters.x` matches the column name `x`
> directly. The two-argument form `sqlpp::parameter(col, name_tag)` is only
> needed when the same column appears as a parameter more than once in a single
> statement and the two occurrences must be distinguishable.

---

## 5. Optional: arithmetic operators

If you want to use arithmetic on custom type columns — in `SET` expressions or
computed `WHERE` clauses — specialise `arithmetic_data_type` and provide the
corresponding `operator` overloads in `namespace sqlpp`.

The example below allows `XCoord + XCoord → XCoord` and
`integral * XCoord → XCoord`:

```cpp
namespace sqlpp {

// XCoord + XCoord → XCoord
template <>
struct arithmetic_data_type<plus, XCoord, XCoord> {
  using type = XCoord;
};
template <typename L, typename R>
  requires(is_xcoord_v<L> and is_xcoord_v<R>)
constexpr auto operator+(L l, R r) -> arithmetic_expression<L, plus, R> {
  return {std::move(l), std::move(r)};
}

// integral * XCoord → XCoord  (e.g. scale a coordinate by a factor)
template <>
struct arithmetic_data_type<multiplies, integral, XCoord> {
  using type = XCoord;
};
template <typename L, typename R>
  requires(is_integral<L>::value and is_xcoord_v<R>)
constexpr auto operator*(L l, R r) -> arithmetic_expression<L, multiplies, R> {
  return {std::move(l), std::move(r)};
}

}  // namespace sqlpp
```

With these in place, arithmetic expressions compile and produce an `XCoord`
result type that can be used in `SET` or `WHERE`:

```cpp
// Double every x coordinate, shift every y by a fixed offset
db(update(t).set(
    t.x = 2 * t.x,
    t.y = t.y + YCoord{17}
));
```

Each operator overload is independent — add only the combinations that make
sense for your domain. Mixed-type arithmetic (e.g. `XCoord + YCoord`) is a
compile error unless you explicitly specialise it.

---

## Reference: complete checklist

| What | Where | Required for |
|---|---|---|
| `to_sql_string(Context&, const T&)` | near `T` (ADL) | All queries |
| `read_field(const Result&, size_t, T&)` | near `T` (ADL) | SELECT result rows |
| `bind_parameter(Statement&, size_t, const T&)` | near `T` (ADL) | Prepared statements |
| `sqlpp::data_type_of<T>` with `type = T` | `namespace sqlpp` | All queries |
| `sqlpp::result_data_type_of<T>` with `type = T` | `namespace sqlpp` | SELECT result rows |
| `sqlpp::parameter_value<T>` with `type = T` | `namespace sqlpp` | Prepared statements |
| `sqlpp::values_are_assignable<L, R>` | `namespace sqlpp` | SET / INSERT |
| `sqlpp::values_are_comparable<L, R>` | `namespace sqlpp` | WHERE, ORDER BY |
| `is_T` / `is_T_v` helper | near `T` | Constraint expressions |
| `sqlpp::arithmetic_data_type<Op, L, R>` + `operator` | `namespace sqlpp` | Arithmetic in queries *(optional)* |

The complete working example can be found in
[`tests/core/usage/CustomType.cpp`](/tests/core/usage/CustomType.cpp).

[**\< Recipes**](/docs/recipes.md)
