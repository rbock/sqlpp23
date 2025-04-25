[**\< Differences**](/docs/differences-to-sqlpp11.md)

# Before and after: Constraints

## Clauses

<table>
<tr>
<th align="left">sqlpp11</th><th align="left">sqlpp23</th>
</tr>
</tr>
<tr><td colspan=2>

  **Unconditional select, update, and delete**

</td></tr>
<tr>
<td  valign="top">

```c++
select(tab.id).from(tab).unconditionally();
```

</td>
<td valign="top">

```c++
select(tab.id).from(tab);
```

</td>
</tr>
<tr>
<td  valign="top">

```c++
update(tab)
    .set(tab.version = 11)
    .unconditionally();
```

</td>
<td valign="top">

```c++
update(tab).set(tab.version = 11);
```

</td>
</tr>
<tr>
<td  valign="top">

```c++
remove_from(tab).unconditionally();
```

</td>
<td valign="top">

```c++
delete_from(tab); // works
truncate(tab); // preferred
```

</td>
</tr>
</table>

## Joins

<table>
<tr>
<th align="left">sqlpp11</th><th align="left">sqlpp23</th>
</tr>
</tr>
<tr><td colspan=2>

  **Inner Joins**

</td></tr>
<tr>
<td  valign="top">

```c++
foo.join(bar).unconditionally();
foo.inner_join(bar).unconditionally();
// or
foo.cross_join(bar);
```

</td>
<td valign="top">

```c++
foo.cross_join(bar);
```

</td>
</tr>
<tr>
<tr><td colspan=2>

  **Outer Joins**

</td></tr>
<tr>
<td  valign="top">

```c++
foo.left_outer_join(bar).unconditionally();
foo.right_outer_join(bar).unconditionally();
foo.outer_join(bar).unconditionally();
```

</td>
<td valign="top">

```c++
// No such thing.
```

</td>
</tr>
<tr>
</table>

[**\< Differences**](/docs/differences-to-sqlpp11.md)

