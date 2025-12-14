# Copyright (c) 2021, Leon De Andrade
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

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

function(add_core)
    # The core library needs the core headers plus all the headers in the top include directory
    file(GLOB_RECURSE HDR_COMPONENT LIST_DIRECTORIES false ${PROJECT_SOURCE_DIR}/include/sqlpp23/core/*.h)
    file(GLOB HDR_COMMON LIST_DIRECTORIES false ${PROJECT_SOURCE_DIR}/include/sqlpp23/*.h)
    set(HEADERS ${HDR_COMPONENT} ${HDR_COMMON})
    add_common(
        CONFIG_SCRIPT Sqlpp23Config.cmake
        HEADERS ${HEADERS}
        TARGET_NAME sqlpp23
        TARGET_ALIAS sqlpp23::core
        TARGET_EXPORTED core
    )
endfunction()

function(add_component)
    set(options)
    set(oneValueArgs HEADER_DIR NAME PACKAGE)
    set(multiValueArgs DEFINES DEPENDENCIES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    file(GLOB_RECURSE HEADERS LIST_DIRECTORIES false ${PROJECT_SOURCE_DIR}/include/sqlpp23/${ARG_HEADER_DIR}/*.h)
    string(TOLOWER ${ARG_NAME} LC_NAME)
    add_common(
        CONFIG_SCRIPT Sqlpp23${ARG_NAME}Config.cmake
        DEFINES ${ARG_DEFINES}
        DEPENDENCIES ${ARG_DEPENDENCIES}
        HEADERS ${HEADERS}
        PACKAGE ${ARG_PACKAGE}
        TARGET_NAME sqlpp23_${LC_NAME}
        TARGET_ALIAS sqlpp23::${LC_NAME}
        TARGET_EXPORTED ${LC_NAME}
    )

endfunction()

function(add_common)
    set(options)
    set(oneValueArgs CONFIG_SCRIPT PACKAGE TARGET_NAME TARGET_ALIAS TARGET_EXPORTED)
    set(multiValueArgs DEFINES DEPENDENCIES HEADERS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # If the component needs a specific package, check if it is installed
    if(ARG_PACKAGE AND DEPENDENCY_CHECK)
        find_package(${ARG_PACKAGE} REQUIRED)
    endif()

    # Create the component targets
    add_library(${ARG_TARGET_NAME} INTERFACE)
    add_library(${ARG_TARGET_ALIAS} ALIAS ${ARG_TARGET_NAME})
    set_target_properties(${ARG_TARGET_NAME} PROPERTIES EXPORT_NAME ${ARG_TARGET_EXPORTED})
    target_compile_features(${ARG_TARGET_NAME} INTERFACE cxx_std_23)
    if(ARG_DEFINES)
        target_compile_definitions(${ARG_TARGET_NAME} INTERFACE ${ARG_DEFINES})
    endif()
    if(ARG_DEPENDENCIES)
        target_link_libraries(${ARG_TARGET_NAME} INTERFACE sqlpp23 ${ARG_DEPENDENCIES})
    endif()
    # Add the component headers to the HEADERS file set. This also adds the base directory to the
    # target's build interface include directories.
    target_sources(
        ${ARG_TARGET_NAME}
        INTERFACE
        FILE_SET HEADERS BASE_DIRS ${PROJECT_SOURCE_DIR}/include FILES ${ARG_HEADERS}
    )

    # Install the component
    install(
        TARGETS ${ARG_TARGET_NAME}
        EXPORT Sqlpp23Targets
        FILE_SET HEADERS
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(
        FILES ${PROJECT_SOURCE_DIR}/cmake/configs/${ARG_CONFIG_SCRIPT}
        DESTINATION ${SQLPP23_INSTALL_CMAKEDIR}
    )
    if(ARG_PACKAGE)
        set(FIND_SCRIPT ${PROJECT_SOURCE_DIR}/cmake/modules/Find{ARG_PACKAGE}.cmake)
        if(EXISTS ${FIND_SCRIPT})
            install(FILES ${FIND_SCRIPT} DESTINATION ${SQLPP23_INSTALL_CMAKEDIR})
        endif()
    endif()
endfunction()
