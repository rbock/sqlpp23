# Copyright (c) 2025, Roland Bock
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice, this
#    list of conditions and the following disclaimer in the documentation and/or
#    other materials provided with the distribution.
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

# Core library module
add_library(sqlpp23.core.module)
target_sources(sqlpp23.core.module
  PUBLIC
    FILE_SET all_my_modules TYPE CXX_MODULES FILES
    sqlpp23.core.cppm
)
target_link_libraries(sqlpp23.core.module PUBLIC sqlpp23)

add_library(sqlpp23.mock_db.module)
target_sources(sqlpp23.mock_db.module
  PUBLIC
    FILE_SET all_my_modules TYPE CXX_MODULES FILES
    sqlpp23.mock_db.cppm
)
target_link_libraries(sqlpp23.mock_db.module PUBLIC sqlpp23.core.module)

if(BUILD_SQLITE3_CONNECTOR OR BUILD_SQLCIPHER_CONNECTOR)
    add_library(sqlpp23.sqlite3.module)
    target_sources(sqlpp23.sqlite3.module
      PUBLIC
        FILE_SET all_my_modules TYPE CXX_MODULES FILES
        sqlpp23.sqlite3.cppm
    )
    target_link_libraries(sqlpp23.sqlite3.module PUBLIC sqlpp23.core.module sqlpp23::sqlite3)
endif()

if(BUILD_MYSQL_CONNECTOR OR BUILD_MARIADB_CONNECTOR)
    add_library(sqlpp23.mysql.module)
    target_sources(sqlpp23.mysql.module
      PUBLIC
        FILE_SET all_my_modules TYPE CXX_MODULES FILES
        sqlpp23.mysql.cppm
    )
    target_link_libraries(sqlpp23.mysql.module PUBLIC sqlpp23.core.module sqlpp23::mysql)
endif()

if(BUILD_POSTGRESQL_CONNECTOR)
    add_library(sqlpp23.postgresql.module)
    target_sources(sqlpp23.postgresql.module
      PUBLIC
        FILE_SET all_my_modules TYPE CXX_MODULES FILES
        sqlpp23.postgresql.cppm
    )
    target_link_libraries(sqlpp23.postgresql.module PUBLIC sqlpp23.core.module sqlpp23::postgresql)
endif()
