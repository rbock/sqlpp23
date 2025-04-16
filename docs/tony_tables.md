[**\< Index**](README.md)

# Tony tables

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
<td colspan=2>

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
</table>

[**\< Index**](README.md)

