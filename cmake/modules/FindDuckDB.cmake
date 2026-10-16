# Copyright (c) 2026
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# DuckDB does not necessarily install a CMake package (the prebuilt release
# archives do not). This module therefore takes the following approach:
#
#   1. If a DuckDB CMake package is available, use it (find_package(DuckDB)).
#   2. Otherwise look for the header and the import library / shared library
#      directly, honouring the usual DuckDB_DIR / DUCKDB_ROOT hints.
#
# In both cases the result is a `duckdb::duckdb` target.

if(TARGET duckdb::duckdb)
    set(DuckDB_FOUND TRUE)
    return()
endif()

# 1) Prefer a real DuckDB package when one is installed.
if(NOT DuckDB_FIND_QUIETLY)
    find_package(DuckDB CONFIG QUIET)
endif()

if(TARGET duckdb)
    # The package config exports an unnamespaced `duckdb` target.
    add_library(duckdb::duckdb ALIAS duckdb)
    set(DuckDB_FOUND TRUE)
    return()
endif()

if(TARGET duckdb_static)
    add_library(duckdb::duckdb ALIAS duckdb_static)
    set(DuckDB_FOUND TRUE)
    return()
endif()

# 2) Fall back to searching for the prebuilt archive layout.
set(_duckdb_hints)
if(DuckDB_DIR)
    list(APPEND _duckdb_hints "${DuckDB_DIR}")
endif()
if(DUCKDB_ROOT)
    list(APPEND _duckdb_hints "${DUCKDB_ROOT}")
endif()
if(DEFINED ENV{DUCKDB_ROOT})
    list(APPEND _duckdb_hints "$ENV{DUCKDB_ROOT}")
endif()
list(APPEND _duckdb_hints
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/duckdb/1.5.6"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/duckdb/1.5.6"
)

find_path(DuckDB_INCLUDE_DIR
    NAMES duckdb.h
    HINTS ${_duckdb_hints}
    PATH_SUFFIXES include
)

find_library(DuckDB_LIBRARY
    NAMES duckdb
    HINTS ${_duckdb_hints}
    PATH_SUFFIXES lib lib64 bin
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    DuckDB
    DEFAULT_MSG
    DuckDB_INCLUDE_DIR
    DuckDB_LIBRARY
)

if(DuckDB_FOUND AND NOT TARGET duckdb::duckdb)
    if(WIN32 AND DuckDB_LIBRARY MATCHES "\\.lib$")
        add_library(duckdb::duckdb SHARED IMPORTED)
        set_target_properties(duckdb::duckdb PROPERTIES
            IMPORTED_IMPLIB "${DuckDB_LIBRARY}"
        )
    else()
        add_library(duckdb::duckdb UNKNOWN IMPORTED)
        set_target_properties(duckdb::duckdb PROPERTIES
            IMPORTED_LOCATION "${DuckDB_LIBRARY}"
        )
    endif()
    set_target_properties(duckdb::duckdb PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${DuckDB_INCLUDE_DIR}"
    )

    set(DuckDB_LIBRARIES duckdb::duckdb)
    set(DuckDB_INCLUDE_DIRS "${DuckDB_INCLUDE_DIR}")
endif()

mark_as_advanced(DuckDB_INCLUDE_DIR DuckDB_LIBRARY)
