/*
 * Copyright (c) 2025, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sqlpp23/tests/mysql/all.h>
#include <sqlpp23/core/reader.h>
#include <sqlpp23/core/logic.h>

namespace sql = sqlpp::mysql;

// Example implementation of MySQL's
// TIMESTAMPDIFF(unit,datetime_expr1,datetime_expr2)
// See
// https://dev.mysql.com/doc/refman/9.3/en/date-and-time-functions.html#function_timestampdiff
//
// See also
// /docs/recipes/custom_function.md
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

namespace {
SQLPP_CREATE_NAME_TAG(difference);
}

// Testing the whole thing
int main(int, char*[]) {
  try {
    auto db = sql::make_test_connection();
    auto ctx = sql::context_t{&db};

    auto a = cast("2001-01-01T00:00:00", as(sqlpp::timestamp{}));
    auto b = cast("2002-01-01T00:00:00", as(sqlpp::timestamp{}));

    for (const auto [unit, expected_diff] : {
             std::pair{example::timestamp_unit::year, 1},
             std::pair{example::timestamp_unit::month, 12},
             std::pair{example::timestamp_unit::day, 365},
             std::pair{example::timestamp_unit::hour, 365 * 24},
             std::pair{example::timestamp_unit::minute, 365 * 24 * 60},
             std::pair{example::timestamp_unit::second, 365 * 24 * 60 * 60},
         }) {
      for (const auto& row :
           db(select(example::timestampdiff(unit, a, b).as(difference)))) {
        if (row.difference != expected_diff) {
          throw std::runtime_error(std::format(
              "Unexpected diff ({}) for unit ({})",
              row.difference ? std::to_string(*row.difference) : "NULL",
              sqlpp::to_sql_string(ctx, unit)));
        }
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
