/*
 * Copyright (c) 2015-2015, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include "Sample.h"
#include "MockDb.h"
#include <sqlpp11/sqlpp11.h>

#include "../../include/test_helpers.h"

SQLPP_CREATE_NAME_TAG(now)

#if _MSC_FULL_VER >= 190023918
// MSVC Update 2 provides floor, ceil, round, abs in chrono (which is C++17 only...)
using ::std::chrono::floor;
#else
using ::date::floor;
#endif

int DateTime(int, char*[])
{
  MockDb db = {};
  MockDb::_serializer_context_t printer = {};
  const auto t = test::TabDateTime{};

  for (const auto& row : db(select(::sqlpp::value(std::chrono::system_clock::now()).as(now))))
  {
    std::cout << row.now;
  }
  for (const auto& row : db(select(all_of(t)).from(t).unconditionally()))
  {
    std::cout << row.dayPointN;
    std::cout << row.timePointN;
    const auto tp = std::chrono::system_clock::time_point{row.timePointN.value()};
    std::cout << std::chrono::system_clock::to_time_t(tp);
  }
  printer.reset();
  std::cerr << to_sql_string(printer, ::sqlpp::value(std::chrono::system_clock::now())).str() << std::endl;

  db(insert_into(t).set(t.dayPointN = floor<::sqlpp::chrono::days>(std::chrono::system_clock::now())));
  db(insert_into(t).set(t.timePointN = floor<::sqlpp::chrono::days>(std::chrono::system_clock::now())));
  db(insert_into(t).set(t.timePointN = std::chrono::system_clock::now()));
  db(insert_into(t).set(t.timeOfDayN = ::sqlpp::chrono::time_of_day(std::chrono::system_clock::now())));

  db(update(t)
         .set(t.dayPointN = floor<::sqlpp::chrono::days>(std::chrono::system_clock::now()))
         .where(t.dayPointN < std::chrono::system_clock::now()));
  db(update(t)
         .set(t.timePointN = floor<::sqlpp::chrono::days>(std::chrono::system_clock::now()),
              t.timeOfDayN = ::sqlpp::chrono::time_of_day(std::chrono::system_clock::now()))
         .where(t.dayPointN < std::chrono::system_clock::now()));
  db(update(t)
         .set(t.timePointN = std::chrono::system_clock::now(),
              t.timeOfDayN = ::sqlpp::chrono::time_of_day(std::chrono::system_clock::now()))
         .where(t.dayPointN < std::chrono::system_clock::now()));

  db(delete_from(t).where(t.dayPointN == floor<::sqlpp::chrono::days>(std::chrono::system_clock::now())));
  db(delete_from(t).where(t.dayPointN == std::chrono::system_clock::now()));
  db(delete_from(t).where(t.timePointN == floor<::sqlpp::chrono::days>(std::chrono::system_clock::now())));
  db(delete_from(t).where(t.timePointN == std::chrono::system_clock::now()));
  db(delete_from(t).where(t.timeOfDayN == ::sqlpp::chrono::time_of_day(std::chrono::system_clock::now())));

  return 0;
}
