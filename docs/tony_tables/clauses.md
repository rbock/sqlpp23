[**\< Differences**](/docs/differences_to_sqlpp11.md)

# Before and after: Clauses

## `SELECT`

<table>
<tr>
<th align="left">sqlpp11</th><th align="left">sqlpp23</th>
</tr>
</tr>
<tr><td colspan=2>

  **using flags**

</td></tr>
<tr>
<td  valign="top">

```c++
// Either
select()
    .flags(sqlpp::all)
    .columns(tab.id)
    .from(tab);
// Or
select(tab.id)
    .flags(sqlpp::all)
    .from(tab);
```

</td>
<td valign="top">

```c++
select(sqlpp::all, tab.id)
    .from(tab);
```

</td>
</tr>
</table>

## `DELETE FROM`

<table>
<tr>
<th align="left">sqlpp11</th><th align="left">sqlpp23</th>
</tr>
</tr>
<tr><td colspan=2>

  **delete from**

</td></tr>
<tr>
<td  valign="top">

```c++
remove_from(tab).where(tab.id == 11);
```

</td>
<td valign="top">

```c++
delete_from(tab).where(tab.id == 11);
```

</td>
</tr>
<tr><td colspan=2>

  **truncate**

</td></tr>
<tr>
<td  valign="top">

```c++
remove_from(tab).unconditionally();
```

</td>
<td valign="top">

```c++
truncate(tab);
```

</td>
</tr>
</table>

## `WITH`

<table>
<tr>
<th align="left">sqlpp11</th><th align="left">sqlpp23</th>
</tr>
</tr>
<tr><td colspan=2>

  **with ... select**

</td></tr>
<tr>
<td  valign="top">

```c++
const auto a = get_my_cte();

for (const auto& row :
     db(with(a)(select(a.intN).from(a)))) {
  // use row.intN
}
```

</td>
<td valign="top">

```c++
const auto a = get_my_cte();

for (const auto& row :
     db(with(a) << select(a.intN).from(a))) {
  // use row.intN
}
```

</td>
</tr>
</table>

## Custom queries

Custom queries can be used to construct statements that the library does not support directly.

<table>
<tr>
<th align="left">sqlpp11</th><th align="left">sqlpp23</th>
</tr>
</tr>
<tr><td colspan=2>

  **select ... into**

</td></tr>
<tr>
<td  valign="top">

```c++
custom_query(select(all_of(foo)).from(foo),
             into(bar))
    .with_result_type_of(insert_into(bar));
```

</td>
<td valign="top">

```c++
select(all_of(t)).from(t)
    << into(f)
    << with_result_type_of(insert_into(f));
```

</td>
</tr>
</table>

[**\< Differences**](/docs/differences_to_sqlpp11.md)

