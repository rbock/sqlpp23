# Copyright (c) 2021, Roland Bock
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
    set(LIB_TYPE)
    set(LIB_PROP_SCOPE PUBLIC)
else()
    set(LIB_TYPE INTERFACE)
    set(LIB_PROP_SCOPE INTERFACE)
endif()
add_library(sqlpp23_core_testing ${LIB_TYPE})
target_include_directories(sqlpp23_core_testing ${LIB_PROP_SCOPE} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(sqlpp23_core_testing ${LIB_PROP_SCOPE} sqlpp23_testing)
if(BUILD_WITH_MODULES)
    target_sources(
        sqlpp23_core_testing
        PUBLIC
            FILE_SET CXX_MODULES FILES modules/sqlpp23.test.core.tables.cppm
    )
    target_link_libraries(
        sqlpp23_core_testing
        PUBLIC
            sqlpp23::core_module
            sqlpp23::mock_db_module
    )
endif()

add_subdirectory(asserts)
add_subdirectory(constraints)
add_subdirectory(helpers)
add_subdirectory(serialize)
add_subdirectory(types)
add_subdirectory(usage)
add_subdirectory(meta)
