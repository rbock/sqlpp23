#include <sqlpp26/select.h>
#include <sqlpp26/alias_provider.h>

int main()
{
  select(sqlpp::value(false).as<"a">());
  return 0;
}
