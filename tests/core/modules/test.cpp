import sqlpp23;

#include <sqlpp23/tests/core/tables.h>

int main() {
  constexpr auto foo = test::TabFoo{};
  where(foo.id > 17);
}
