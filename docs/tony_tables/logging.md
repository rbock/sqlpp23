[**\< Differences**](/docs/differences-to-sqlpp11.md)

# Before and after: Debug logging

Turning on debug logging got slightly more complicated is also much more flexible.

<table>
<tr>
<th align="left">sqlpp11</th><th align="left">sqlpp23</th>
</tr>
</tr>
<tr><td colspan=2>

  **Turn off logging**

</td></tr>
<tr>
<td  valign="top">

```c++
config->debug = false;
```

</td>
<td valign="top">

```c++
config->debug = {};
```

</td>
</tr>
<tr><td colspan=2>

  **Turn on logging (all messages to stderr)**

</td></tr>
<tr>
<td  valign="top">

```c++
config->debug = true;
```

</td>
<td valign="top">

```c++
constexpr auto log_cerr =
    [](const std::string& message) {
      std::cerr << message << '\n';
    };

// ...

config->debug = sqlpp::debug_logger{
    {sqlpp::log_category::all},
    log_cerr};
```

</td>
</tr>
<tr><td colspan=2>

  **Turn on logging (some messages to clog)**

</td></tr>
<tr>
<td  valign="top">

```c++
// Not possible.
```

</td>
<td valign="top">

```c++
constexpr auto log_clog =
    [](const std::string& message) {
      std::clog << message << '\n';
    };

// ...

// Log messages about statements and their
// parameters to std::clog.
config->debug = sqlpp::debug_logger{
    {sqlpp::log_category::statement,
     sqlpp::log_category::parameter},
    log_clog};
```

</td>
</tr>
<tr><td colspan=2>

  **Turn off logging at compile time**

</td></tr>
<tr>
<td  valign="top">

```c++
// Not possible.
```

</td>
<td valign="top">

```c++
// Before including sqlpp23/core/debug_logger.h in
// the current compilation unit.
#define SQLPP23_DISABLE_DEBUG
```

</td>
</tr>
</table>

[**\< Differences**](/docs/differences-to-sqlpp11.md)

