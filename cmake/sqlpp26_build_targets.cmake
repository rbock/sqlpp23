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

function(add_build_core)
    # The core library needs the core headers plus all the headers in the top include directory
    file(GLOB_RECURSE hdr_component LIST_DIRECTORIES false ${PROJECT_SOURCE_DIR}/include/sqlpp26/core/*.h)
    file(GLOB hdr_common LIST_DIRECTORIES false ${PROJECT_SOURCE_DIR}/include/sqlpp26/*.h)
    set(headers ${hdr_component} ${hdr_common})
    _add_build_regular_and_module(
        CONFIG_SCRIPT Sqlpp26Config.cmake
        HEADERS ${headers}
        MODULE_INTERFACE sqlpp26.core.cppm
        TARGET_NAME sqlpp26
        TARGET_ALIAS sqlpp26::core
        TARGET_EXPORTED core
    )
endfunction()

function(add_build_component)
    set(options NO_INSTALL)
    set(oneValueArgs HEADER_DIR MODULE_INTERFACE NAME PACKAGE)
    set(multiValueArgs DEFINES DEPENDENCIES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    file(GLOB_RECURSE headers LIST_DIRECTORIES false ${PROJECT_SOURCE_DIR}/include/sqlpp26/${ARG_HEADER_DIR}/*.h)
    string(TOLOWER ${ARG_NAME} lc_name)
    _add_build_set_if(NO_INSTALL ARG_NO_INSTALL)
    _add_build_regular_and_module(
        CONFIG_SCRIPT Sqlpp26${ARG_NAME}Config.cmake
        DEFINES ${ARG_DEFINES}
        DEPENDENCIES sqlpp26::core ${ARG_DEPENDENCIES}
        HEADERS ${headers}
        MODULE_INTERFACE ${ARG_MODULE_INTERFACE}
        PACKAGE ${ARG_PACKAGE}
        TARGET_NAME sqlpp26_${lc_name}
        TARGET_ALIAS sqlpp26::${lc_name}
        TARGET_EXPORTED ${lc_name}
        ${NO_INSTALL}
    )
endfunction()

function(_add_build_regular_and_module)
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
            set(find_script ${PROJECT_SOURCE_DIR}/cmake/modules/Find${ARG_PACKAGE}.cmake)
            if(EXISTS ${find_script})
                install(FILES ${find_script} DESTINATION ${SQLPP26_INSTALL_CMAKEDIR})
            endif()
        endif()
    endif()
    if(NOT ARG_NO_INSTALL)
        install(
            FILES ${PROJECT_SOURCE_DIR}/cmake/configs/${ARG_CONFIG_SCRIPT}
            DESTINATION ${SQLPP26_INSTALL_CMAKEDIR}
        )
    endif()
    _add_build_set_if(NO_INSTALL ARG_NO_INSTALL)
    _add_build_common(
        DEFINES ${ARG_DEFINES}
        DEPENDENCIES ${ARG_DEPENDENCIES}
        HEADERS ${ARG_HEADERS}
        TARGET_NAME ${ARG_TARGET_NAME}
        TARGET_ALIAS ${ARG_TARGET_ALIAS}
        TARGET_EXPORTED ${ARG_TARGET_EXPORTED}
        ${NO_INSTALL}
    )
    if(BUILD_WITH_MODULES AND ARG_MODULE_INTERFACE)
        _add_build_common(
            DEFINES ${ARG_DEFINES}
            DEPENDENCIES ${ARG_DEPENDENCIES}
            HEADERS ${ARG_HEADERS}
            MODULE_INTERFACE ${ARG_MODULE_INTERFACE}
            TARGET_NAME ${ARG_TARGET_NAME}
            TARGET_ALIAS ${ARG_TARGET_ALIAS}
            TARGET_EXPORTED ${ARG_TARGET_EXPORTED}
            ${NO_INSTALL}
        )
    endif()
endfunction()

function(_add_build_common)
    set(options NO_INSTALL)
    set(oneValueArgs MODULE_INTERFACE TARGET_NAME TARGET_ALIAS TARGET_EXPORTED)
    set(multiValueArgs DEFINES DEPENDENCIES HEADERS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Initialize helper variables based on target type (regular or module)
    if(ARG_MODULE_INTERFACE)
        set(target_suffix "_module")
        # CMake has the following two limitations:
        # - FILE_SETs of type CXX_MODULES cannot have the INTERFACE scope (except on IMPORTED targets).
        # - INTERFACE libraries only allow INTERFACE scope on their properties.
        # From these two limitations it follows that INTERFACE libraries cannot have FILE_SETs of type
        # CXX_MODULES. That's why, as a workaround, we use an OBJECT library.
        #
        # For details see the discussion at
        # https://discourse.cmake.org/t/header-only-libraries-and-c-20-modules/10680
        # where the CMake devs explain that this limitation exists to prevent possible ODR violations.
        set(lib_type OBJECT)
        set(lib_prop_scope PUBLIC)
    else()
        set(target_suffix "")
        set(lib_type INTERFACE)
        set(lib_prop_scope INTERFACE)
    endif()
    set(target_name ${ARG_TARGET_NAME}${target_suffix})
    set(target_alias ${ARG_TARGET_ALIAS}${target_suffix})
    set(target_exported ${ARG_TARGET_EXPORTED}${target_suffix})

    # Create the component targets
    add_library(${target_name} ${lib_type})
    add_library(${target_alias} ALIAS ${target_name})
    set_target_properties(${target_name} PROPERTIES EXPORT_NAME ${target_exported})
    target_compile_features(${target_name} ${lib_prop_scope} cxx_std_26)
    if(ARG_DEFINES)
        target_compile_definitions(${target_name} ${lib_prop_scope} ${ARG_DEFINES})
    endif()
    foreach(dep ${ARG_DEPENDENCIES})
        if(dep MATCHES "^sqlpp26::")
            set(dep ${dep}${target_suffix})
        endif()
        target_link_libraries(${target_name} ${lib_prop_scope} ${dep})
    endforeach()

    # Add the component headers to the HEADERS file set. This also adds the base directory to the
    # target's build interface include directories.
    target_sources(
        ${target_name}
        ${lib_prop_scope}
        FILE_SET HEADERS
            BASE_DIRS ${PROJECT_SOURCE_DIR}/include
            FILES ${ARG_HEADERS}
    )
    if(ARG_MODULE_INTERFACE)
        # Add the component module interface file to the CXX_MODULES file set
        target_sources(
            ${target_name}
            PUBLIC
            FILE_SET CXX_MODULES
                BASE_DIRS ${PROJECT_SOURCE_DIR}/modules
                FILES ${PROJECT_SOURCE_DIR}/modules/${ARG_MODULE_INTERFACE}
        )
    endif()
    if(NOT ARG_NO_INSTALL)
        # Install the component output artifacts
        install(
            TARGETS ${target_name}
            EXPORT Sqlpp26Targets
            FILE_SET HEADERS
            FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_PREFIX}/modules/sqlpp26
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
    endif()
endfunction()

# Helper macro that is used to forward option arguments in function calls
# Taken from https://stackoverflow.com/a/75994425/5689371
macro(_add_build_set_if option condition)
    if(${condition})
        set(${option} "${option}")
    else()
        set(${option})
    endif()
endmacro()
