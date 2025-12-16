# Copyright (c) 2021-2021, Roland Bock
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

if(BUILD_WITH_MODULES)
  add_library(sqlpp23.test.postgresql.tables.module)
  target_sources(sqlpp23.test.postgresql.tables.module
    PUBLIC
      FILE_SET all_my_modules TYPE CXX_MODULES FILES
      modules/sqlpp23.test.postgresql.tables.cppm
  )
  target_link_libraries(sqlpp23.test.postgresql.tables.module PUBLIC sqlpp23::core_module)
endif()

add_library(sqlpp23_postgresql_testing INTERFACE)
target_include_directories(sqlpp23_postgresql_testing INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(BUILD_WITH_MODULES)
    target_link_libraries(sqlpp23_postgresql_testing INTERFACE
      sqlpp23_testing
      sqlpp23::postgresql_module
      sqlpp23.test.postgresql.tables.module
    )
else()
    target_link_libraries(sqlpp23_postgresql_testing INTERFACE
      sqlpp23_testing
    )
endif()

add_subdirectory(asserts)
add_subdirectory(constraints)
add_subdirectory(recipes)
add_subdirectory(serialize)
add_subdirectory(types)
add_subdirectory(usage)
