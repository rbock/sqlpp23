#pragma once

/*
 * Copyright (c) 2013, Roland Bock
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

#define SQLPP_NAME_TAG_GUTS(SQL_NAME, CPP_NAME)                    \
  [[maybe_unused]] static constexpr bool require_quotes = false;   \
  [[maybe_unused]] static constexpr const char name[] = #SQL_NAME; \
  template <typename T>                                            \
  struct _member_t {                                               \
    T CPP_NAME = {};                                               \
    auto& operator()(this auto&& self) {                           \
      return self.CPP_NAME;                                        \
    }                                                              \
  }

#define SQLPP_CREATE_NAME_TAG_FOR_SQL_AND_CPP(SQL_NAME, CPP_NAME) \
  struct _sqlpp_name_tag {                                        \
    SQLPP_NAME_TAG_GUTS(SQL_NAME, CPP_NAME);                      \
  }

#define SQLPP_CREATE_NAME_TAG(NAME)                    \
  struct NAME##_t {                                    \
    SQLPP_CREATE_NAME_TAG_FOR_SQL_AND_CPP(NAME, NAME); \
  };                                                   \
  inline constexpr auto NAME = NAME##_t {}

#define SQLPP_QUOTED_NAME_TAG_GUTS(SQL_NAME, CPP_NAME) \
  static constexpr bool require_quotes = true;         \
  static constexpr const char name[] = #SQL_NAME;      \
  template <typename T>                                \
  struct _member_t {                                   \
    T CPP_NAME = {};                                   \
    auto& operator()(this auto&& self) {               \
      return self.CPP_NAME;                            \
    }                                                  \
  }

#define SQLPP_CREATE_QUOTED_NAME_TAG_FOR_SQL_AND_CPP(SQL_NAME, CPP_NAME) \
  struct _sqlpp_name_tag {                                               \
    SQLPP_QUOTED_NAME_TAG_GUTS(SQL_NAME, CPP_NAME);                      \
  }

#define SQLPP_CREATE_QUOTED_NAME_TAG(NAME)                    \
  struct NAME##_t {                                           \
    SQLPP_CREATE_QUOTED_NAME_TAG_FOR_SQL_AND_CPP(NAME, NAME); \
  };                                                          \
  inline constexpr auto NAME = NAME##_t {}

