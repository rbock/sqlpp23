#include <sqlpp23/core/chrono.h>
#include <sqlpp23/core/type_traits.h>
#include <sqlpp23/core/to_sql_string.h>
#include <string>
#include <vector>
#include <cstdint>

namespace my_ns {
struct uuid {
  bool operator==(const uuid&) const = default;
};

template <typename T>
struct is_uuid : public std::is_same<sqlpp::remove_optional_t<sqlpp::data_type_of_t<T>>, uuid> {};

template <typename T>
inline constexpr bool is_uuid_v = is_uuid<T>::value;

template <typename Context>
auto to_sql_string(Context& context, const uuid& v) -> std::string {
  return "''";
}

template <typename Result>
void read_field(const Result& result, size_t index, uuid& v) {
}

template <typename Statement>
void bind_parameter(Statement& stmt, size_t index, const uuid& v) {
}

}  // namespace my_ns

namespace sqlpp {
template <>
struct data_type_of<my_ns::uuid> {
  using type = my_ns::uuid;
};

template <>
struct result_data_type_of<my_ns::uuid> {
  using type = my_ns::uuid;
};

template <>
struct parameter_value<my_ns::uuid> {
  using type = my_ns::uuid;
};

template <typename L, typename R>
  requires(my_ns::is_uuid_v<L> and my_ns::is_uuid_v<R>)
struct values_are_assignable<L, R> : public std::true_type {};

template <typename L, typename R>
  requires(my_ns::is_uuid_v<L> and my_ns::is_uuid_v<R>)
struct values_are_comparable<L, R> : public std::true_type {};

} // namespace sqlpp

#include <ddl2cpp_sample_good_custom_type_old.h>
#include <ddl2cpp_sample_good_custom_type_new.h>

template <typename T>
void test_db_model() {
  T tab_foo;
  tab_foo.myBoolean = true;
  tab_foo.myInteger = 5;
  tab_foo.mySerial = 10;
  tab_foo.myFloatingPoint = 12.34;
  tab_foo.myText = "test";
  tab_foo.myBlob = std::vector<uint8_t>{'b', 'l', 'o', 'b'};
  tab_foo.myDate = std::chrono::sys_days{};
  tab_foo.myDateTime = std::chrono::system_clock::now();
  tab_foo.myTime = std::chrono::seconds{10};
  // Special cases
  tab_foo.mySecondText = "another text";
  tab_foo.myTypeWithSpaces = 20;
  // Capitalisation
  tab_foo.capBoolean = false;
  // Build in types
  tab_foo.builtinBoolean = true;
  tab_foo.builtinInteger = 5;
  tab_foo.builtinSerial = 10;
  tab_foo.builtinFloatingPoint = 12.34;
  tab_foo.builtinText = "test";
  tab_foo.builtinBlob = std::vector<uint8_t>{'b', 'l', 'o', 'b'};
  tab_foo.builtinDate = std::chrono::sys_days{};
  tab_foo.builtinDateTime = std::chrono::system_clock::now();
  tab_foo.builtinTime = std::chrono::seconds{10};

  // Assignment test
  [[maybe_unused]] auto assign_uuid = (tab_foo.myUuid = my_ns::uuid{});
}

int main() {
  test_db_model<test::dbm_old::TabFoo>();
  test_db_model<test::dbm_new::TabFoo>();
}
