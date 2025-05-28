[**< Index**](/docs/README.md)

# Data types

## Nullable types

Nullablility is expressed using `std::optional`. `NULL` itself is expressed as `std::nullopt`.

## Bool

SQL `BOOL` is represented by `sqlpp::boolean`, e.g. in column specifications.

Parameters and result fields use the C++ type `bool`.

For assignments and comparisons any numeric type will work.

## Integer

SQL `INTEGER`, `INT`, `BIGINT`, etc, are collectively represented by `sqlpp::integral`, e.g. in column specifications.

Parameters and result fields use the C++ type `int64_t`.

C++ data types `int8_t`, `int16_t`, `int32_t`, and `int64_t` are considered `sqlpp::integral`.

For assignments and comparisons any numeric type will work.

## Unsigned integer

SQL `UNSIGNED INTEGER`, `UNSIGNED INT`, `UNSIGNED BIGINT`, etc, are collectively represented by `sqlpp::unsigned_integral`, e.g. in column specifications.

Parameters and result fields use the C++ type `uint64_t`.

C++ data types `uint8_t`, `uint16_t`, `uint32_t`, and `uint64_t` are considered `sqlpp::unsigned_integral`.

For assignments and comparisons any numeric type will work.

## Floating Point

SQL `FLOAT`, `DOUBLE`, etc, are collectively represented by `sqlpp::floating_point`, e.g. in column specifications.

Parameters and result fields use the C++ type `double`.

C++ data types `float`, `double`, `long double` are considered `sqlpp::floating_point`.

For assignments and comparisons any numeric type will work.

## Text

SQL `CHAR`, `VARCHAR`, `TEXT`, etc, are collectively represented by `sqlpp::text`, e.g. in column specifications.

Parameters use the C++ type `std::string` and result fields use the C++ type `std::string_view`.

C++ data types `char`, `const char*`, `std::string`, and `std::string_view` are considered `sqlpp::text`.

For assignments and comparisons expressions of type `sqlpp::text` will work.

## Blob

SQL `BLOB`, `BIGBLOB`, etc, are collectively represented by `sqlpp::blob`, e.g. in column specifications.

Parameters use the C++ type `std::vector<uint8_t>` and result fields use the C++ type `std::span<uint8_t>`.

C++ data types `std::array<uint8_t>`, `std::span<uint8_t>`, and `std::vector<uint8_t>` are considered `sqlpp::blob`.

For assignments and comparisons expressions of type `sqlpp::text` will work.

## Date, Timestamp, etc

Note that sqlpp23 ignores timezones. It works best if both the database and the C++ code operate in UTC. If you are doing anything else, you are on your own.

### Date

SQL `DATE` is represented by `sqlpp::date`, e.g. in column specifications.

Parameters and result fields use the C++ type `std::chrono::sys_days`.

C++ data types `std::chrono::sys_days` is considered `sqlpp::date`.

For assignments expressions of type `sqlpp::date` will work. Comparisons additionally work with `sqlpp::timestamp`.

### Timestamp

SQL `TIMESTAMP` is represented by `sqlpp::timestamp`, e.g. in column specifications.

Parameters and result fields use the C++ type `std::chrono::sys_time<std::chrono::microseconds>>`.

C++ data types `std::chrono::sys_time<Period>>` is considered `sqlpp::timestamp` if `Period` is smaller than `std::chrono::days`.

For assignments, expressions of type `sqlpp::timestamp` will work. Comparisons addtionally work with `sqlpp::date`.

### Time of day

SQL `TIME` is represented by `sqlpp::time_of_day`, e.g. in column specifications.

Parameters and result fields use the C++ type `std::chrono::microseconds`.

C++ data types `std::chrono::duration<Rep, Period>` is considered `sqlpp::time_of_day`.

For assignments and comparisons expressions of type `sqlpp::time_of_day` will work.

[**< Index**](/docs/README.md)
