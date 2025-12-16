# Copyright (c) 2013-2021, Roland Bock
# Copyright (c) 2016 Christian Dávid
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
#   Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
#   Redistributions in binary form must reproduce the above copyright notice, this
#   list of conditions and the following disclaimer in the documentation and/or
#   other materials provided with the distribution.
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

### Preamble
cmake_minimum_required(VERSION 3.28)
project(sqlpp23 VERSION 0.1 LANGUAGES CXX)

### Project-wide setup
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

include(GNUInstallDirs)
include(CTest)
include(Sqlpp23TargetHelper)

option(BUILD_MYSQL_CONNECTOR "Build MySQL Connector" OFF)
option(BUILD_MARIADB_CONNECTOR "Build MariaDB Connector" OFF)
option(BUILD_POSTGRESQL_CONNECTOR "Build PostgreSQL Connector" OFF)
option(BUILD_SQLITE3_CONNECTOR "Build SQLite3 Connector" OFF)
option(BUILD_SQLCIPHER_CONNECTOR "Build SQLite3 Connector with SQLCipher" OFF)
option(BUILD_WITH_MODULES "Build tests with sqlpp23 modules" OFF)

option(DEPENDENCY_CHECK "Check for dependencies of connector and the library" ON)

set(SQLPP23_INSTALL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/Sqlpp23 CACHE STRING "Path to sqlpp23 cmake files")

# If we are building without C++20 modules, don't scan the sources for module dependencies
# This speeds up the build process.
if(NOT BUILD_WITH_MODULES)
    set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
endif()

### Main (core) library
add_core()

### Mock Database
add_component(NAME mock_db HEADER_DIR mock_db MODULE_INTERFACE sqlpp23.mock_db.cppm NO_INSTALL)

### Connector components
if(BUILD_SQLITE3_CONNECTOR)
    add_component(NAME SQLite3 PACKAGE SQLite3 DEPENDENCIES SQLite::SQLite3 HEADER_DIR sqlite3 MODULE_INTERFACE sqlpp23.sqlite3.cppm)
endif()
if(BUILD_SQLCIPHER_CONNECTOR)
    add_component(NAME SQLCipher PACKAGE SQLCipher DEPENDENCIES SQLCipher::SQLCipher DEFINES SQLPP_USE_SQLCIPHER HEADER_DIR sqlite3 MODULE_INTERFACE sqlpp23.sqlite3.cppm)
endif()
if(BUILD_MYSQL_CONNECTOR)
    add_component(NAME MySQL PACKAGE MySQL DEPENDENCIES MySQL::MySQL HEADER_DIR mysql MODULE_INTERFACE sqlpp23.mysql.cppm)
endif()
if(BUILD_MARIADB_CONNECTOR)
    add_component(NAME MariaDB PACKAGE MariaDB DEPENDENCIES MariaDB::MariaDB HEADER_DIR mysql MODULE_INTERFACE sqlpp23.mysql.cppm)
endif()
if(BUILD_POSTGRESQL_CONNECTOR)
    add_component(NAME PostgreSQL PACKAGE PostgreSQL DEPENDENCIES PostgreSQL::PostgreSQL HEADER_DIR postgresql MODULE_INTERFACE sqlpp23.postgresql.cppm)
endif()

# Utility programs
install(PROGRAMS ${PROJECT_SOURCE_DIR}/scripts/sqlpp23-ddl2cpp
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

### Packaging
write_basic_package_version_file(Sqlpp23ConfigVersion.cmake
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Sqlpp23ConfigVersion.cmake
    DESTINATION ${SQLPP23_INSTALL_CMAKEDIR}
)
install(EXPORT Sqlpp23Targets
    DESTINATION ${SQLPP23_INSTALL_CMAKEDIR}
    NAMESPACE   sqlpp23::
    CXX_MODULES_DIRECTORY .
)

### Tests
if(PROJECT_IS_TOP_LEVEL AND BUILD_TESTING)
    add_subdirectory(tests)
endif()
