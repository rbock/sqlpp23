[**\< Recipes**](/docs/recipes.md)

# Add a custom SQL function

sqlpp23 source code can be daunting due to the intense use of templates.

However, adding an unsupported SQL function to your project is pretty straight forward.

The example below show-cases a non-trivial example of a function with multiple parameters,
including a custom enum. Single argument functions like
[`TRIM()`](/include/sqlpp23/core/function/trim.h) are considerably simpler.

## Step 1: Define the API

```c++
// Example implementation of MySQL's
// TIMESTAMPDIFF(unit,datetime_expr1,datetime_expr2)
// See
// https://dev.mysql.com/doc/refman/9.3/en/date-and-time-functions.html#function_timestampdiff
namespace example {
enum class timestamp_unit { year, month, day, hour, minute, second };

// Helper to access the `_unit` data member.
struct reader_t {
  template <typename T>
  const auto& unit(const T& t) const {
    return t._unit;
  }
};

inline constexpr auto read = reader_t{};

// timestampdiff_t represents TIMESTAMPDIFF(unit,lhs,rhs)
// The base classes provide comparison functions (e.g. <, .in(), .is_null()) and
// `.as()`.
template <typename Lhs, typename Rhs>
class timestampdiff_t : public sqlpp::enable_comparison,
                        public sqlpp::enable_as {
 public:
  timestampdiff_t(timestamp_unit unit, Lhs lhs, Rhs rhs)
      : _unit{unit}, _lhs{std::move(lhs)}, _rhs{std::move(rhs)} {}

  timestampdiff_t(const timestampdiff_t&) = default;
  timestampdiff_t(timestampdiff_t&&) = default;
  timestampdiff_t& operator=(const timestampdiff_t&) = default;
  timestampdiff_t& operator=(timestampdiff_t&&) = default;
  ~timestampdiff_t() = default;

  // Making the data private is not really required, but it reduces the API
  // surface.
 private:
  friend sqlpp::reader_t; // This has a bunch of accessor functions.
  friend example::reader_t; // This is used to provide access to `_unit`.

  timestamp_unit _unit;
  Lhs _lhs;
  Rhs _rhs;
};

// The function users are supposed to call.
template <typename Lhs, typename Rhs>
  requires((sqlpp::is_timestamp<Lhs>::value or sqlpp::is_date<Lhs>::value) and
           (sqlpp::is_timestamp<Rhs>::value or sqlpp::is_date<Rhs>::value))
auto timestampdiff(timestamp_unit unit, Lhs lhs, Rhs rhs) {
  return timestampdiff_t{unit, std::move(lhs), std::move(rhs)};
}
}  // namespace example

```

## Step 2: Connect your new function with the rest of the library:

```c++
// For making `example::timestampdiff_t` work in `sqlpp` expressions, a couple
// of template specializations are required:
namespace sqlpp {

// This specifies the data type of `example::timestampdiff_t`.
template <typename Lhs, typename Rhs>
struct data_type_of<example::timestampdiff_t<Lhs, Rhs>> {
  using type = std::conditional_t<
      logic::any<is_optional<data_type_of_t<Lhs>>::value,
                 is_optional<data_type_of_t<Rhs>>::value>::value,
      std::optional<sqlpp::integral>,
      sqlpp::integral>;
};

// Declaring nodes is required for
// * tracking parameters (Lhs or Rhs could contain parameters)
// * analyzing query correctness, e.g. Lhs or Rhs could refer to tables that are not provided in the `FROM` clause.
template <typename Lhs, typename Rhs>
struct nodes_of<example::timestampdiff_t<Lhs, Rhs>> {
  using type = detail::type_vector<Lhs, Rhs>;
};

// This turns `unit` into a string.
template <typename Context>
auto to_sql_string(Context&, example::timestamp_unit unit) {
  switch (unit) {
    case example::timestamp_unit::year:
      return "YEAR";
    case example::timestamp_unit::month:
      return "MONTH";
    case example::timestamp_unit::day:
      return "DAY";
    case example::timestamp_unit::hour:
      return "HOUR";
    case example::timestamp_unit::minute:
      return "MINUTE";
    case example::timestamp_unit::second:
      return "SECOND";
  };
}

// This turns a `example::timestampdiff_t` into a string. It accesses to data members through
// the `read` objects.
template <typename Context, typename Lhs, typename Rhs>
auto to_sql_string(Context& context,
                   const example::timestampdiff_t<Lhs, Rhs>& t) -> std::string {
  // In case the context needs to keep track of something (like the index of
  // parameters in PostgrSQL), it is required to ensure that data members are
  // serialized left ot right. Since the evaluation order for function arguments
  // is undefined, one way is to serialize them before the call to `format`.
  auto lhs = to_sql_string(context, read.lhs(t));
  auto rhs = to_sql_string(context, read.rhs(t));
  return std::format("TIMESTAMPDIFF({}, {}, {})",
                     to_sql_string(context, example::read.unit(t)), std::move(lhs),
                     std::move(rhs));
}

}  // namespace sqlpp

```

## Step 3: Use it

```c++
SQLPP_CREATE_NAME_TAG(difference); // Outside of function

// ...

auto a = cast("2001-01-01T00:00:00", as(sqlpp::timestamp{}));
auto b = cast("2002-01-01T00:00:00", as(sqlpp::timestamp{}));
auto unit = example::timestamp_unit::month;

for (const auto& row :
     db(select(example::timestampdiff(unit, a, b).as(difference)))) {
    do_something(row.difference);
}
```

The complete source code can be found [here](/tests/mysql/recipes/custom_function.cpp).


[**\< Recipes**](/docs/recipes.md)

