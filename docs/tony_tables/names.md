[**\< Differences**](/docs/differences-to-sqlpp11.md)

# Before and after: Names

<table>
<tr>
<th align="left">sqlpp11</th><th align="left">sqlpp23</th>
</tr>
</tr>
<tr><td colspan=2>

  **Names**

</td></tr>
<tr>
<td  valign="top">

```c++
SQLPP_ALIAS_PROVIDER(something)
```

</td>
<td valign="top">

```c++
SQLPP_CREATE_NAME_TAG(something);
```

</td>
</tr>
<tr><td colspan=2>

  **Quoted Names**

</td></tr>
<tr>
<td  valign="top">

```c++
SQLPP_QUOTED_ALIAS_PROVIDER(something)
```

</td>
<td valign="top">

```c++
SQLPP_CREATE_QUOTED_NAME_TAG(something);
```

</td>
</tr>
</table>

[**\< Differences**](/docs/differences-to-sqlpp11.md)

