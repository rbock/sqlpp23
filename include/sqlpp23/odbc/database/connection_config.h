#pragma once

/*
 * Copyright (c) 2026, Leander Schulten
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
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

#include <cstddef>
#include <string>
#include <variant>

#include <sqlpp23/core/debug_logger.h>

namespace sqlpp::odbc {
// Connect to a data source that is configured in the ODBC driver manager
// (e.g. in odbc.ini), using SQLConnect.
struct data_source {
  std::string name;
  std::string username;
  std::string password;

  bool operator==(const data_source&) const = default;
};

// Connect using a complete ODBC connection string, e.g.
//   "Driver=PostgreSQL Unicode;Server=localhost;Database=test;Uid=me;Pwd=pw;"
// using SQLDriverConnect (without prompting).
struct connection_string {
  std::string value;

  bool operator==(const connection_string&) const = default;
};

struct connection_config {
  // There is exactly one way to connect per configuration: either through a
  // configured data source or through a connection string.
  std::variant<data_source, connection_string> source;

  // Number of rows fetched per driver round trip when a select result can be
  // bound to fixed-size buffers (see max_bound_column_size). 1 disables
  // multi-row fetch.
  std::size_t row_array_size{64};

  // Per-row buffer limit (in bytes) for binding text and blob result columns.
  // Columns whose maximum size is unknown or would exceed this limit make the
  // result fall back to fetching one row at a time (streaming each value via
  // SQLGetData), which has no size limit.
  std::size_t max_bound_column_size{4096};

  debug_logger debug;  // not compared

  bool operator==(const connection_config& other) const {
    return source == other.source and row_array_size == other.row_array_size and
           max_bound_column_size == other.max_bound_column_size;
  }
};
}  // namespace sqlpp::odbc
