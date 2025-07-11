import sqlpp23_postgresql;
#include <tuple>

#include <sqlpp23/tests/core/tables.h>

int main() {
  constexpr auto foo = test::TabFoo{};
  auto db = sqlpp::postgresql::connection{};
  for (const auto& row : db(select(all_of(foo)).from(foo))) {
    std::ignore = row.id;
  }
}
