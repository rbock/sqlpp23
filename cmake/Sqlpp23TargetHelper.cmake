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

# Helper macro that is used to forward option arguments in function calls
# Taken from https://stackoverflow.com/a/75994425/5689371
macro(set_if OPTION CONDITION)
    if(${CONDITION})
        set(${OPTION} "${OPTION}")
    else()
        set(${OPTION})
    endif()
endmacro()

function(add_core)
    # The core library needs the core headers plus all the headers in the top include directory
    file(GLOB_RECURSE HDR_COMPONENT LIST_DIRECTORIES false ${PROJECT_SOURCE_DIR}/include/sqlpp23/core/*.h)
    file(GLOB HDR_COMMON LIST_DIRECTORIES false ${PROJECT_SOURCE_DIR}/include/sqlpp23/*.h)
    set(HEADERS ${HDR_COMPONENT} ${HDR_COMMON})
    add_regular_and_module(
        CONFIG_SCRIPT Sqlpp23Config.cmake
        HEADERS ${HEADERS}
        MODULE_INTERFACE sqlpp23.core.cppm
        TARGET_NAME sqlpp23
        TARGET_ALIAS sqlpp23::core
        TARGET_EXPORTED core
    )
endfunction()

function(add_component)
    set(options NO_INSTALL)
    set(oneValueArgs HEADER_DIR MODULE_INTERFACE NAME PACKAGE)
    set(multiValueArgs DEFINES DEPENDENCIES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    file(GLOB_RECURSE HEADERS LIST_DIRECTORIES false ${PROJECT_SOURCE_DIR}/include/sqlpp23/${ARG_HEADER_DIR}/*.h)
    string(TOLOWER ${ARG_NAME} LC_NAME)
    set_if(NO_INSTALL ARG_NO_INSTALL)
    add_regular_and_module(
        CONFIG_SCRIPT Sqlpp23${ARG_NAME}Config.cmake
        DEFINES ${ARG_DEFINES}
        DEPENDENCIES sqlpp23::core ${ARG_DEPENDENCIES}
        HEADERS ${HEADERS}
        MODULE_INTERFACE ${ARG_MODULE_INTERFACE}
        PACKAGE ${ARG_PACKAGE}
        TARGET_NAME sqlpp23_${LC_NAME}
        TARGET_ALIAS sqlpp23::${LC_NAME}
        TARGET_EXPORTED ${LC_NAME}
        ${NO_INSTALL}
    )
endfunction()

function(add_regular_and_module)
    set(options NO_INSTALL)
    set(oneValueArgs CONFIG_SCRIPT MODULE_INTERFACE PACKAGE TARGET_NAME TARGET_ALIAS TARGET_EXPORTED)
    set(multiValueArgs DEFINES DEPENDENCIES HEADERS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(ARG_PACKAGE)
        if(DEPENDENCY_CHECK)
            find_package(${ARG_PACKAGE} REQUIRED)
        endif()
        if(NOT ARG_NO_INSTALL)
            # If the package needs a special find script, copy it to the destination scripts directory
            set(FIND_SCRIPT ${PROJECT_SOURCE_DIR}/cmake/modules/Find{ARG_PACKAGE}.cmake)
            if(EXISTS ${FIND_SCRIPT})
                install(FILES ${FIND_SCRIPT} DESTINATION ${SQLPP23_INSTALL_CMAKEDIR})
            endif()
        endif()
    endif()
    if(NOT ARG_NO_INSTALL)
        install(
            FILES ${PROJECT_SOURCE_DIR}/cmake/configs/${ARG_CONFIG_SCRIPT}
            DESTINATION ${SQLPP23_INSTALL_CMAKEDIR}
        )
    endif()
    set_if(NO_INSTALL ARG_NO_INSTALL)
    add_common(
        DEFINES ${ARG_DEFINES}
        DEPENDENCIES ${ARG_DEPENDENCIES}
        HEADERS ${HEADERS}
        TARGET_NAME ${ARG_TARGET_NAME}
        TARGET_ALIAS ${ARG_TARGET_ALIAS}
        TARGET_EXPORTED ${ARG_TARGET_EXPORTED}
        ${NO_INSTALL}
    )
    if(BUILD_WITH_MODULES AND ARG_MODULE_INTERFACE)
        add_common(
            DEFINES ${ARG_DEFINES}
            DEPENDENCIES ${ARG_DEPENDENCIES}
            HEADERS ${HEADERS}
            MODULE_INTERFACE ${ARG_MODULE_INTERFACE}
            TARGET_NAME ${ARG_TARGET_NAME}
            TARGET_ALIAS ${ARG_TARGET_ALIAS}
            TARGET_EXPORTED ${ARG_TARGET_EXPORTED}
            ${NO_INSTALL}
        )
    endif()
endfunction()

function(add_common)
    set(options NO_INSTALL)
    set(oneValueArgs MODULE_INTERFACE TARGET_NAME TARGET_ALIAS TARGET_EXPORTED)
    set(multiValueArgs DEFINES DEPENDENCIES HEADERS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Initialize helper variables based on target type (regular or module)
    if(ARG_MODULE_INTERFACE)
        set(TARGET_SUFFIX "_module")
        # FILE_SETs of type CXX_MODULES cannot have the INTERFACE scope (exept
        # on IMPORTED targets) and INTERFACE libraries only allow INTERFACE
        # scope on their properties. That's why we cannot use the INTERFACE
        # library type. For details see the discussion at
        # https://discourse.cmake.org/t/header-only-libraries-and-c-20-modules/10680/11
        set(LIB_TYPE OBJECT)
        set(LIB_PROP_SCOPE PUBLIC)
    else()
        set(TARGET_SUFFIX "")
        set(LIB_TYPE INTERFACE)
        set(LIB_PROP_SCOPE INTERFACE)
    endif()
    set(TARGET_NAME ${ARG_TARGET_NAME}${TARGET_SUFFIX})
    set(TARGET_ALIAS ${ARG_TARGET_ALIAS}${TARGET_SUFFIX})
    set(TARGET_EXPORTED ${ARG_TARGET_EXPORTED}${TARGET_SUFFIX})

    # Create the component targets
    add_library(${TARGET_NAME} ${LIB_TYPE})
    add_library(${TARGET_ALIAS} ALIAS ${TARGET_NAME})
    set_target_properties(${TARGET_NAME} PROPERTIES EXPORT_NAME ${TARGET_EXPORTED})
    target_compile_features(${TARGET_NAME} ${LIB_PROP_SCOPE} cxx_std_23)
    if(ARG_DEFINES)
        target_compile_definitions(${TARGET_NAME} ${LIB_PROP_SCOPE} ${ARG_DEFINES})
    endif()
    foreach(DEP ${ARG_DEPENDENCIES})
        if(DEP MATCHES "^sqlpp23::")
            set(DEP ${DEP}${TARGET_SUFFIX})
        endif()
        target_link_libraries(${TARGET_NAME} ${LIB_PROP_SCOPE} ${DEP})
    endforeach()

    # Add the component headers to the HEADERS file set. This also adds the base directory to the
    # target's build interface include directories.
    target_sources(
        ${TARGET_NAME}
        ${LIB_PROP_SCOPE}
        FILE_SET HEADERS
            BASE_DIRS ${PROJECT_SOURCE_DIR}/include
            FILES ${ARG_HEADERS}
    )
    if(ARG_MODULE_INTERFACE)
        # Add the component module interface file to the CXX_MODULES file set
        target_sources(
            ${TARGET_NAME}
            PUBLIC
            FILE_SET CXX_MODULES
                BASE_DIRS ${PROJECT_SOURCE_DIR}/modules
                FILES ${PROJECT_SOURCE_DIR}/modules/${ARG_MODULE_INTERFACE}
        )
    endif()
    if(NOT ARG_NO_INSTALL)
        # Install the component output artifacts
        install(
            TARGETS ${TARGET_NAME}
            EXPORT Sqlpp23Targets
            FILE_SET HEADERS
            FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_PREFIX}/modules/sqlpp23
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
    endif()
endfunction()
