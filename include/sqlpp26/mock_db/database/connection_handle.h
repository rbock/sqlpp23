#pragma once

/*
 * Copyright (c) 2025, Roland Bock
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

#include <memory>

#include <sqlpp23/core/database/exception.h>
#include <sqlpp23/mock_db/database/connection_config.h>

class MockDb {};

inline void mockdb_close(MockDb*) {}

namespace sqlpp::mock_db::detail {
struct connection_handle {
  std::shared_ptr<const connection_config> config;
  std::unique_ptr<MockDb, void (*)(MockDb*)> _mockdb;

  connection_handle() : config{}, _mockdb{nullptr, mockdb_close} {}

  connection_handle(const std::shared_ptr<const connection_config>& conf)
      : config{conf}, _mockdb{new MockDb, mockdb_close} {
    if (not _mockdb) {
      throw sqlpp::exception{"MockDb: could not init MockDb data structure"};
    }
  }

  connection_handle(const connection_handle&) = delete;
  connection_handle(connection_handle&&) = default;
  connection_handle& operator=(const connection_handle&) = delete;
  connection_handle& operator=(connection_handle&&) = default;

  MockDb* native_handle() const { return _mockdb.get(); }

  bool is_connected() const {
    return native_handle();
  }

  bool ping_server() const {
    return native_handle();
  }

  const debug_logger& debug() { return config->debug; }
};
}  // namespace sqlpp::mock_db::detail
