#pragma once

/**
 * Copyright © 2014-2015, Matthijs Möhlmann
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

#include <memory>
#include <string>

#include <libpq-fe.h>

#include <sqlpp23/postgresql/database/connection_config.h>
#include <sqlpp23/postgresql/database/exception.h>

namespace sqlpp::postgresql::detail {
struct connection_handle {
  std::shared_ptr<const connection_config> config;
  std::unique_ptr<PGconn, void (*)(PGconn*)> postgres;
  size_t _prepared_statement_count = 0;

  connection_handle() : config{}, postgres{nullptr, PQfinish} {}

  connection_handle(const std::shared_ptr<const connection_config>& conf)
      : config{conf}, postgres{nullptr, PQfinish} {
    if constexpr (debug_enabled) {
      config->debug.log(log_category::connection,
                        "connecting to the database server.");
    }

    // Open connection
    std::string conninfo = "";
    if (!config->host.empty()) {
      conninfo.append("host=" + config->host);
    }
    if (!config->hostaddr.empty()) {
      conninfo.append(" hostaddr=" + config->hostaddr);
    }
    if (config->port != 5432) {
      conninfo.append(" port=" + std::to_string(config->port));
    }
    if (!config->dbname.empty()) {
      conninfo.append(" dbname=" + config->dbname);
    }
    if (!config->user.empty()) {
      conninfo.append(" user=" + config->user);
    }
    if (!config->password.empty()) {
      conninfo.append(" password=" + config->password);
    }
    if (config->connect_timeout != 0) {
      conninfo.append(" connect_timeout=" +
                      std::to_string(config->connect_timeout));
    }
    if (!config->client_encoding.empty()) {
      conninfo.append(" client_encoding=" + config->client_encoding);
    }
    if (!config->options.empty()) {
      conninfo.append(" options=" + config->options);
    }
    if (!config->application_name.empty()) {
      conninfo.append(" application_name=" + config->application_name);
    }
    if (!config->fallback_application_name.empty()) {
      conninfo.append(" fallback_application_name=" +
                      config->fallback_application_name);
    }
    if (!config->keepalives) {
      conninfo.append(" keepalives=0");
    }
    if (config->keepalives_idle != 0) {
      conninfo.append(" keepalives_idle=" +
                      std::to_string(config->keepalives_idle));
    }
    if (config->keepalives_interval != 0) {
      conninfo.append(" keepalives_interval=" +
                      std::to_string(config->keepalives_interval));
    }
    if (config->keepalives_count != 0) {
      conninfo.append(" keepalives_count=" +
                      std::to_string(config->keepalives_count));
    }
    switch (config->sslmode) {
      case connection_config::sslmode_t::disable:
        conninfo.append(" sslmode=disable");
        break;
      case connection_config::sslmode_t::allow:
        conninfo.append(" sslmode=allow");
        break;
      case connection_config::sslmode_t::require:
        conninfo.append(" sslmode=require");
        break;
      case connection_config::sslmode_t::verify_ca:
        conninfo.append(" sslmode=verify-ca");
        break;
      case connection_config::sslmode_t::verify_full:
        conninfo.append(" sslmode=verify-full");
        break;
      case connection_config::sslmode_t::prefer:
        break;
    }
    if (!config->sslcompression) {
      conninfo.append(" sslcompression=0");
    }
    if (!config->sslcert.empty()) {
      conninfo.append(" sslcert=" + config->sslcert);
    }
    if (!config->sslkey.empty()) {
      conninfo.append(" sslkey=" + config->sslkey);
    }
    if (!config->sslrootcert.empty()) {
      conninfo.append(" sslrootcert=" + config->sslrootcert);
    }
    if (!config->requirepeer.empty()) {
      conninfo.append(" requirepeer=" + config->requirepeer);
    }
    if (!config->krbsrvname.empty()) {
      conninfo.append(" krbsrvname=" + config->krbsrvname);
    }
    if (!config->service.empty()) {
      conninfo.append(" service=" + config->service);
    }

    postgres.reset(PQconnectdb(conninfo.c_str()));

    if (is_connected() == false) {
      throw connection_exception{PQerrorMessage(native_handle())};
    }
  }

  connection_handle(const connection_handle&) = delete;
  connection_handle(connection_handle&&) = default;

  ~connection_handle() {
    // Debug
    if constexpr (debug_enabled) {
      if (is_connected()) {
        config->debug.log(log_category::connection,
                          "closing database connection.");
      }
    }
  }

  connection_handle& operator=(const connection_handle&) = delete;
  connection_handle& operator=(connection_handle&&) = default;

  std::string get_prepared_statement_name() {
    ++_prepared_statement_count;
    return std::to_string(_prepared_statement_count);
  }

  PGconn* native_handle() const { return postgres.get(); }

  bool is_connected() const {
    return native_handle() and (PQstatus(native_handle()) == CONNECTION_OK);
  }

  bool ping_server() const {
    // Loosely based on the implementation of PHP's pg_ping()
    if (is_connected() == false) {
      return false;
    }
    auto exec_res = PQexec(native_handle(), "SELECT 1");
    auto exec_ok = PQresultStatus(exec_res) == PGRES_TUPLES_OK;
    PQclear(exec_res);
    return exec_ok;
  }

  const debug_logger& debug() { return config->debug; }
};
}  // namespace sqlpp::postgresql::detail
