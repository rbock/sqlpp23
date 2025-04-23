[**\< Differences**](../differences-to-sqlpp11.md)

# Before and after: dynamic queries

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

[**\< Differences**](../differences-to-sqlpp11.md)

