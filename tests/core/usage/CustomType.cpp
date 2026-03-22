/*
 * Copyright (c) 2026, Roland Bock
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

#include <sqlpp23/tests/core/all.h>

// This test introduces two data types that represent X/Y coordinates, e.g. of a
// point.
// By telling the library which fields represent XCoord and which represent
// YCoord, we can ensure that these values are not accidentally mixed. In order
// to do so, we also need to introduce a few helpers to make sqlpp23 understand
// how to handle these custom types.
namespace {
// This represents an X-coordinate.
struct XCoord
{
   int64_t value;
};
// Syntactic sugar for testing whether a type is XCoord.
template <typename T>
struct is_xcoord : public std::is_same<sqlpp::data_type_of_t<T>, XCoord> {};
template <typename T>
static constexpr bool is_xcoord_v = is_xcoord<T>::value;

// Helper to serialize an XCoord to SQL (same as the encapsulated value).
template <typename Context>
auto to_sql_string(Context& context, const XCoord& xcoord) {
  using sqlpp::to_sql_string;
  return to_sql_string(context, xcoord.value);
}

// Helper to read an XCoord from the result set (again, same as the value).
template <typename Result>
void read_field(const Result& result, size_t field_index, XCoord& xcoord) {
  read_field(result, field_index, xcoord.value);
}
}  // namespace

namespace sqlpp {
// Specialize data_type_of to make is_xcoord work.
template <>
struct data_type_of<XCoord> {
  using type = XCoord;
};

// Specializing record_data_type_of is required for sqlpp23 to know the field
// type in the result row.
template <>
struct result_data_type_of<XCoord> {
  using type = XCoord;
};

// This says that (only) XCoord can be assigned to XCoord.
template <typename L, typename R>
  requires(is_xcoord_v<L> and is_xcoord_v<R>)
struct values_are_assignable<L, R> : public std::true_type {};

// This says that (only) XCoord can be compared to XCoord.
// It also implicitly enables sorting by XCoord columns.
template <typename L, typename R>
  requires(is_xcoord_v<L> and is_xcoord_v<R>)
struct values_are_comparable<L, R> : public std::true_type {};

// We might also want to allow addition of XCoord values.
template <>
struct arithmetic_data_type<plus, XCoord, XCoord> {
  using type = XCoord;
};
template <typename L, typename R>
  requires(is_xcoord_v<L> and is_xcoord_v<R>)
constexpr auto operator+(L l, R r) -> arithmetic_expression<L, plus, R> {
  return {std::move(l), std::move(r)};
}

// We might also want to allow multiplication of XCoord values by an integral
// value.
template <>
struct arithmetic_data_type<multiplies, integral, XCoord> {
  using type = XCoord;
};
template <typename L, typename R>
  requires(is_integral<L>::value and is_xcoord_v<R>)
constexpr auto operator*(L l, R r) -> arithmetic_expression<L, multiplies, R> {
  return {std::move(l), std::move(r)};
}

}  // namespace sqlpp

namespace {
struct YCoord
{
   int64_t value;
};
template <typename T>
struct is_ycoord : public std::is_same<sqlpp::data_type_of_t<T>, YCoord> {};
template <typename T>
static constexpr bool is_ycoord_v = is_ycoord<T>::value;
template <typename Context>
auto to_sql_string(Context& context, const YCoord& ycoord) {
  using sqlpp::to_sql_string;
  return to_sql_string(context, ycoord.value);
}

template <typename Result>
void read_field(const Result& result, size_t field_index, YCoord& ycoord) {
  read_field(result, field_index, ycoord.value);
}

}  // namespace

namespace sqlpp {
template <>
struct data_type_of<YCoord> {
  using type = YCoord;
};

template <>
struct result_data_type_of<YCoord> {
  using type = YCoord;
};

template <typename L, typename R>
  requires(is_ycoord<L>::value and is_ycoord<R>::value)
struct values_are_assignable<L, R> : public std::true_type {};

template <typename L, typename R>
  requires(is_ycoord<L>::value and is_ycoord<R>::value)
struct values_are_comparable<L, R> : public std::true_type {};

template <>
struct arithmetic_data_type<plus, YCoord, YCoord> {
  using type = YCoord;
};
template <typename L, typename R>
  requires(is_ycoord_v<L> and is_ycoord_v<R>)
constexpr auto operator+(L l, R r) -> arithmetic_expression<L, plus, R> {
  return {std::move(l), std::move(r)};
}

template <>
struct arithmetic_data_type<multiplies, integral, YCoord> {
  using type = YCoord;
};
template <typename L, typename R>
  requires(is_integral<L>::value and is_ycoord_v<R>)
constexpr auto operator*(L l, R r) -> arithmetic_expression<L, multiplies, R> {
  return {std::move(l), std::move(r)};
}

}  // namespace sqlpp

namespace test {
  struct TabPoint_ {
    struct Id {
      SQLPP_CREATE_NAME_TAG_FOR_SQL_AND_CPP(id, id);
      using data_type = ::sqlpp::integral;
      using has_default = std::true_type;
    };
    struct X {
      SQLPP_CREATE_NAME_TAG_FOR_SQL_AND_CPP(x, x);
      using data_type = XCoord;
      using has_default = std::true_type;
    };
    struct Y {
      SQLPP_CREATE_NAME_TAG_FOR_SQL_AND_CPP(y, y);
      using data_type = YCoord;
      using has_default = std::true_type;
    };
    SQLPP_CREATE_NAME_TAG_FOR_SQL_AND_CPP(tab_point, tabPoint);
    template<typename T>
    using _table_columns = sqlpp::table_columns<T,
               Id,
               X,
               Y>;
    using _required_insert_columns = sqlpp::detail::type_set<>;
  };
  using TabPoint = ::sqlpp::table_t<TabPoint_>;

}  // namespace
int CustomType(int, char*[]) {
  sqlpp::mock_db::connection db = sqlpp::mock_db::make_test_connection();

  const auto t = test::TabPoint{};

  // Insert (requires values_are_assignable)
  db(insert_into(t).set(t.x = XCoord{6}, t.y = YCoord{49}));

  // Update (requires operator+, operator*, and arithmetic_data_type)
  db(update(t).set(t.x = 2 * t.x, t.y = t.y + YCoord{17}));

  // Select (requires result_data_type_of and values_are_comparable)
  for (const auto& row : db(select(all_of(t))
                                .from(t)
                                .where(t.x > XCoord{5})
                                .order_by(t.y.asc()))) {
    static_assert(std::is_same_v<decltype(row.x), XCoord>);
    static_assert(std::is_same_v<decltype(row.y), YCoord>);
  }
  return 0;
}
