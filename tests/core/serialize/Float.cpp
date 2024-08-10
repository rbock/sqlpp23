/*
 * Copyright (c) 2023, Roland Bock
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

#include <sqlpp11/sqlpp11.h>

#include "compare.h"

namespace
{
  template <typename Result, typename Expected>
  void assert_equal(int lineNo, const Result& result, const Expected& expected)
  {
    if (result != expected)
    {
      std::cerr << __FILE__ << " " << lineNo << '\n'
                << "Expected: -->|" << expected << "|<--\n"
                << "Received: -->|" << result << "|<--\n";
      throw std::runtime_error("unexpected result");
    }
  }

  template <typename T>
  void to_sql_string_serializes_in_deserializable_format(int line, T value)
  {
    MockDb::_serializer_context_t printer = {};
    const auto serialized = sqlpp::to_sql_string(printer, value);
    std::istringstream is{serialized};
    T deserialized;
    is >> deserialized;
    assert_equal(line, deserialized, value);
  }

  template <typename T>
  std::string string_for_10_0000086()
  {
    switch (std::numeric_limits<T>::max_digits10)
    {
      case 9:
        return "10.0000086";
      case 17:
        return "10.000008599999999";
      case 21:
        return "10.0000086000000000001";
      default:
        throw std::logic_error("Unknown floating point digit count");
    }
  }
}  // namespace

int Float(int, char*[])
{
#warning: document that connectors need to use float_safe_ostringstream or similar.
  to_sql_string_serializes_in_deserializable_format(__LINE__, 10.0000086f);
  to_sql_string_serializes_in_deserializable_format(__LINE__, 10.0000086);
  to_sql_string_serializes_in_deserializable_format(__LINE__, 10.0000086l);

  SQLPP_COMPARE(10.0000114, "10.0000114");
  SQLPP_COMPARE(10.0000086f, string_for_10_0000086<float>());
  SQLPP_COMPARE(10.0000086, string_for_10_0000086<double>());
  SQLPP_COMPARE(10.0000086l, string_for_10_0000086<long double>());

  return 0;
}
