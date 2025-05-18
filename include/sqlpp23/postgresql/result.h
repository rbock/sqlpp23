#pragma once

/**
 * Copyright © 2015-2016, Bartosz Wieczorek
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
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

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>

#include <libpq-fe.h>
#include <pg_config.h>

#include <sqlpp23/postgresql/database/exception.h>

namespace sqlpp::postgresql {
class Result {
 public:
  Result() : _pg_result(nullptr, PQclear) {}
  explicit Result(PGresult* res) : _pg_result(res, PQclear) { check_status(); }

  PGresult* native_handle() { return _pg_result.get(); }

  ExecStatusType status() { return PQresultStatus(_pg_result.get()); }

  void clear() {
    _pg_result.reset();
  }

  int affected_rows() {
    const char* const rows_str = PQcmdTuples(_pg_result.get());
    return rows_str[0] ? std::stoi(std::string{rows_str}) : 0;
  }

  int records_size() const { return _pg_result.get() ? PQntuples(_pg_result.get()) : 0; }

  int field_count() const { return _pg_result.get() ? PQnfields(_pg_result.get()) : 0; }

  int length(int record, int field) const {
    /// check index?
    return PQgetlength(_pg_result.get(), record, field);
  }

  bool is_null(int record, int field) const {
    /// check index?
    return PQgetisnull(_pg_result.get(), record, field);
  }

  operator bool() const { return _pg_result.get() != 0; }

  int64_t get_int64_value(int record, int field) const {
    check_index(record, field);
    auto t = int64_t{};
    const auto txt = std::string{PQgetvalue(_pg_result.get(), record, field)};
    if (txt != "") {
      t = std::stoll(txt);
    }

    return t;
  }

  uint64_t get_uint64_value(int record, int field) const {
    check_index(record, field);
    auto t = uint64_t{};
    const auto txt = std::string{PQgetvalue(_pg_result.get(), record, field)};
    if (txt != "") {
      t = std::stoull(txt);
    }

    return t;
  }

  double get_double_value(int record, int field) const {
    check_index(record, field);
    auto t = double{};
    auto txt = std::string{PQgetvalue(_pg_result.get(), record, field)};
    if (txt != "") {
      t = std::stod(txt);
    }

    return t;
  }

  const char* get_char_ptr_value(int record, int field) const {
    return const_cast<const char*>(PQgetvalue(_pg_result.get(), record, field));
  }

  std::string get_string_value(int record, int field) const {
    return {get_char_ptr_value(record, field)};
  }

  const uint8_t* get_blob_value(int record, int field) const {
    return reinterpret_cast<const uint8_t*>(
        PQgetvalue(_pg_result.get(), record, field));
  }

  bool get_bool_value(int record, int field) const {
    check_index(record, field);
    auto val = PQgetvalue(_pg_result.get(), record, field);
    if (*val == 't')
      return true;
    else if (*val == 'f')
      return false;
    return const_cast<const char*>(val);
  }

 private:
  void check_status() const {
    const std::string err = status_error();
    if (!err.empty()) {
      throw_sql_error(err);
    }
  }

  [[noreturn]] void throw_sql_error(const std::string& err) const {
    // Try to establish more precise error type, and throw corresponding
    // exception
    const char* const code = PQresultErrorField(_pg_result.get(), PG_DIAG_SQLSTATE);
    if (code)
      switch (code[0]) {
        case '0':
          switch (code[1]) {
            case '8':
              throw broken_connection{err};
            case 'A':
              throw feature_not_supported{err};
          }
          break;
        case '2':
          switch (code[1]) {
            case '2':
              throw data_exception{err};
            case '3':
              if (strcmp(code, "23001") == 0)
                throw restrict_violation{err};
              if (strcmp(code, "23502") == 0)
                throw not_null_violation{err};
              if (strcmp(code, "23503") == 0)
                throw foreign_key_violation{err};
              if (strcmp(code, "23505") == 0)
                throw unique_violation{err};
              if (strcmp(code, "23514") == 0)
                throw check_violation{err};
              throw integrity_constraint_violation{err};
            case '4':
              throw invalid_cursor_state{err};
            case '6':
              throw invalid_sql_statement_name{err};
          }
          break;
        case '3':
          switch (code[1]) {
            case '4':
              throw invalid_cursor_name{err};
          }
          break;
        case '4':
          switch (code[1]) {
            case '0':
              if (strcmp(code, "40001") == 0)
                throw serialization_failure{err};
              if (strcmp(code, "40P01") == 0)
                throw deadlock_detected{err};
              break;
            case '2':
              if (strcmp(code, "42501") == 0)
                throw insufficient_privilege{err};
              if (strcmp(code, "42601") == 0)
                throw syntax_error{err, error_position()};
              if (strcmp(code, "42703") == 0)
                throw undefined_column{err};
              if (strcmp(code, "42883") == 0)
                throw undefined_function{err};
              if (strcmp(code, "42P01") == 0)
                throw undefined_table{err};
          }
          break;
        case '5':
          switch (code[1]) {
            case '3':
              if (strcmp(code, "53100") == 0)
                throw disk_full{err};
              if (strcmp(code, "53200") == 0)
                throw out_of_memory{err};
              if (strcmp(code, "53300") == 0)
                throw too_many_connections{err};
              throw insufficient_resources{err};
          }
          break;

        case 'P':
          if (strcmp(code, "P0001") == 0)
            throw plpgsql_raise{err};
          if (strcmp(code, "P0002") == 0)
            throw plpgsql_no_data_found{err};
          if (strcmp(code, "P0003") == 0)
            throw plpgsql_too_many_rows{err};
          throw plpgsql_error{err};
          break;
        default:
          throw sql_user_error{err, code};
      }
    throw sql_error{err};
  }

  std::string status_error() const {
    if (!_pg_result.get())
      throw failure{"No result set given"};

    std::string err;

    switch (PQresultStatus(_pg_result.get())) {
      case PGRES_EMPTY_QUERY:  // The string sent to the backend was empty.
      case PGRES_COMMAND_OK:  // Successful completion of a command returning no
                              // data
      case PGRES_TUPLES_OK:   // The query successfully executed
        break;

      case PGRES_COPY_OUT:  // Copy Out (from server) data transfer started
      case PGRES_COPY_IN:   // Copy In (to server) data transfer started
        break;

      case PGRES_BAD_RESPONSE:  // The server's response was not understood
      case PGRES_NONFATAL_ERROR:
      case PGRES_FATAL_ERROR:
        err = PQresultErrorMessage(_pg_result.get());
        break;
#if PG_MAJORVERSION_NUM >= 13
      case PGRES_COPY_BOTH:
      case PGRES_SINGLE_TUPLE:
#endif
#if PG_MAJORVERSION_NUM >= 14
      case PGRES_PIPELINE_SYNC:
      case PGRES_PIPELINE_ABORTED:
#endif
      default:
        throw sqlpp::exception{"pqxx::result: Unrecognized response code " +
                               std::to_string(PQresultStatus(_pg_result.get()))};
    }
    return err;
  }

  int error_position() const noexcept {
    int pos = -1;
    if (_pg_result.get()) {
      const char* p = PQresultErrorField(_pg_result.get(), PG_DIAG_STATEMENT_POSITION);
      if (p)
        pos = std::stoi(std::string{p});
    }
    return pos;
  }

  void check_index(int record, int field) const noexcept(false) {
    if (record > records_size() || field > field_count())
      throw std::out_of_range{"PostgreSQL error: index out of range"};
  }

  std::unique_ptr<PGresult, void(*)(PGresult*)> _pg_result;
};
}  // namespace sqlpp::postgresql
