[**< Index**](/docs/README.md)

# Debug logging

## Runtime log configuration

The connection configuration has a member called `debug` or type `sqlpp::debug_logger`.

The default constructed `debug_logger` does not log anything. But the configuration can be changed to log certain or all messages. The constructor takes 2 parameters:

```c++
// enum class log_category : uint8_t {
//   statement = 0x01,   // Preparation and execution of statements.
//   parameter = 0x02,   // The parameters sent with a prepared query.
//   result = 0x04,      // Result fields and rows.
//   connection = 0x08,  // Other connection interactions, e.g. opening, closing.
//   all = 0xFF,
// };
//
// using log_function_t = std::function<void(const std::string&)>;
debug_logger(const std::vector<sqlpp::log_category>& categories,
             sqlpp::log_function_t log_function);
```

Here are a couple of examples:

```c++
// A simple logger to stderr
constexpr auto log_cerr = [](const std::string& message) {
  std::cerr << message << '\n';
};

// Log all messages to stderr.
config->debug = sqlpp::debug_logger{{sqlpp::log_category::all}, log_cerr};

// Log messages about statements and their parameters to stderr.
config->debug = sqlpp::debug_logger{
    {sqlpp::log_category::statement, sqlpp::log_category::parameter},
    log_cerr};
```

## Turning off debug logging at compile time

If the macro `SQLPP23_DISABLE_DEBUG` is defined before `sqlpp23/core/debug_logger.h` gets included, then
all debug logging of the library is turned off at compile time, which leads to smaller binaries and slightly
faster execution.

[**< Index**](/docs/README.md)
