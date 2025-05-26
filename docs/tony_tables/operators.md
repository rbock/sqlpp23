[**\< Differences**](/docs/differences-to-sqlpp11.md)

# Before and after: dynamic queries

## `CASE`

`case_when` now supports multiple `when`/`then` pairs.

<table>
<tr>
<th align="left">sqlpp11</th><th align="left">sqlpp23</th>
</tr>
</tr>
<tr>
<td  valign="top">

```c++
// Simulating multiple when/then pairs with
// nested case_when expressions.
case_when(cond)
    .then(val)
    .else_(case_when(cond2)
               .then(val2)
               .else_(val3));
```

</td>
<td valign="top">

```c++
case_when(cond)
    .then(val)
    .when(cond2)
    .then(val2)
    .else_(val3);
```

</td>
</tr>
</table>

Data types in `then` and `else_` now must be the same (plus/minus optional).

<table>
<tr>
<th align="left">sqlpp11</th><th align="left">sqlpp23</th>
</tr>
</tr>
<tr>
<td  valign="top">

```c++
case_when(cond)
    .then(17)     // integral
    .else_(17.4); // float is comparable.
                  // Overall type: integral.
```

</td>
<td valign="top">

```c++
case_when(cond)
    .then(17)     // integral
    .else_(17.4); // Compile error.
                  // float is not integral.
```

</td>
</tr>
</table>

`std::nullopt` is now allowed as `then` argument as long as either another `then` argument or the `else_` argument has a data type.

<table>
<tr>
<th align="left">sqlpp11</th><th align="left">sqlpp23</th>
</tr>
</tr>
<tr>
<td  valign="top">

```c++
// Need to invert condition and move the
// `sqlpp::null` to the `else_`
case_when(cond)
    .then(sqlpp::null) // Compile error.
    .else_(17.4);
```

</td>
<td valign="top">

```c++
case_when(cond)
    .then(std::nullopt) // This is fine
    .else_(17.4); // Overall data type:
                  // optional<floating_point>
```

</td>
</tr>
</table>



## `IN` / `NOT IN`

`in` and `not_in` remain unchanged for discrete values, e.g. `x.in(1, 2, 3)` or `x.in(my_tuple)`,
but using `std::vector` has become simpler.

<table>
<tr>
<th align="left">sqlpp11</th><th align="left">sqlpp23</th>
</tr>
</tr>
<tr>
<td  valign="top">

```c++
const auto values = std::vector<int>{1, 2, 3};

x.in(sqlpp::value_list(values));

x.not_in(sqlpp::value_list(values));
```
</td>
<td valign="top">

```c++
const auto values = std::vector<int>{1, 2, 3};

x.in(values);

x.not_in(values);
```

</td>
</tr>
</table>

## `IS DISTINCT FROM` and friends

> [!NOTE]
> Different databases have different ways of spelling `IS DISTINCT FROM`.
> The connectors serialize appropriately.

<table>
<tr>
<th align="left">sqlpp11</th><th align="left">sqlpp23</th>
</tr>
<tr><td colspan=2>

  **DISTINCT FROM**

</td></tr>
<tr>
<td  valign="top">

```c++
not is_equal_to_or_null(
    foo.textN,
    sqlpp::value_or_null("SQL"));

not is_equal_to_or_null(
    foo.textN,
    sqlpp::value_or_null<sqlpp::text>(
        sqlpp::null));

// Cannot be used with parameters.
```
</td>
<td valign="top">

```c++
foo.textN.is_distinct_from("SQL");

foo.textN.is_distinct_from(std::nullopt);

// Can be used with parameter
foo.textN.is_distinct_from(parameter(foo.textN));
```

</td>
</tr>
<tr><td colspan=2>

  **NOT DISTINCT FROM**

</td></tr>
<tr>
<td  valign="top">

```c++
is_equal_to_or_null(
    foo.textN,
    sqlpp::value_or_null("SQL"));

is_equal_to_or_null(
    foo.textN,
    sqlpp::value_or_null<sqlpp::text>(
        sqlpp::null));

// Cannot be used with parameters.
```
</td>
<td valign="top">

```c++
foo.textN.is_not_distinct_from("SQL");

foo.textN.is_not_distinct_from(std::nullopt);

// Can be used with parameter
foo.textN.is_not_distinct_from(
    parameter(foo.textN));
```

</td>
</tr>
</table>

[**\< Differences**](/docs/differences-to-sqlpp11.md)

