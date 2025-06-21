[**\< Differences**](/docs/differences_to_sqlpp11.md)

# Before and after: Dynamic queries

## `SELECT`

<table>
<tr>
<th align="left">sqlpp11</th><th align="left">sqlpp23</th>
</tr>
</tr>
<tr><td colspan=2>

  **Dynamically selected column**

</td></tr>
<tr>
<td  valign="top">

```c++
auto s = dynamic_select(db, tab.id)
             .from(tab)
             .unconditionally();
if (maybe) {
  s.selected_columns.add(tab.int_value);
}
for (const auto& row : db(s)) {
  const int64_t id = row.id;
  const std::string need_to_parse =
      row.at("int_value");
}
```
</td>
<td valign="top">

```c++
for (const auto& row :
     db(select(tab.id,
               dynamic(maybe, tab.int_value))
            .from(tab))) {
  const int64_t id = row.id;
  const int64_t = row.int_value;
}
```

</td>
</tr>
</table>

## Dynamic conditions

Conditions can be used in `where`, `having`, for instance. The tables here use `where` as an example.

<table>
<tr>
<th align="left">sqlpp11</th><th align="left">sqlpp23</th>
</tr>
<tr><td colspan=2>

  **Dynamic AND**

</td></tr>
<tr>
<td  valign="top">

```c++
auto s = dynamic_select(db, tab.id)
             .from(tab)
             .dynamic_where(tab.lang == "c++");
if (maybe_23) {
  s.where.add(tab.version >= "23");
}
for (const auto& row : db(s)) {
  do_something(row.id);
}
```
</td>
<td valign="top">

```c++
for (const auto& row :
     db(select(tab.id).from(tab).where(
         tab.lang == "c++" and
         dynamic(maybe_23, tab_version >= 23)))) {
  do_something(row.id);
}
```

</td>
</tr>
<tr><td colspan=2>

  **Dynamic OR**

</td></tr>
<tr>
<td  valign="top">

```c++
// Not supported.
```
</td>
<td valign="top">

```c++
for (const auto& row :
     db(select(tab.id).from(tab).where(
         tab.lang == "c++" or
         dynamic(maybe_23, tab_version >= 23)))) {
  do_something(row.id);
}
```

</td>
</tr>
<tr><td colspan=2>

  **Nested dynamic**

</td></tr>
<tr>
<td  valign="top">

```c++
// Not supported.
```
</td>
<td valign="top">

```c++
for (const auto& row :
     db(select(tab.id).from(tab).where(
         tab.lang == "c++" and
         (tab.legacy == true or
          dynamic(maybe_23,
                  tab_version >= 23))))) {
  do_something(row.id);
}
```

</td>
</tr>
</table>

[**\< Differences**](/docs/differences_to_sqlpp11.md)

