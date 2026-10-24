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
    file(GLOB_RECURSE hdr_component LIST_DIRECTORIES false ${PROJECT_SOURCE_DIR}/include/sqlpp23/core/*.h)
    file(GLOB hdr_common LIST_DIRECTORIES false ${PROJECT_SOURCE_DIR}/include/sqlpp23/*.h)
    set(headers ${hdr_component} ${hdr_common})
    _add_build_common(
        CONFIG_SCRIPT Sqlpp23Config.cmake
        HEADERS ${headers}
        MODULE_INTERFACE sqlpp23.core.cppm
        TARGET_NAME sqlpp23
        TARGET_ALIAS sqlpp23::core
        TARGET_EXPORTED core
        TARGET_FILE Sqlpp23CoreTargets
    )
endfunction()

function(add_build_component)
    set(options NO_INSTALL)
    set(oneValueArgs HEADER_DIR MODULE_INTERFACE NAME PACKAGE)
    set(multiValueArgs DEFINES DEPENDENCIES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    file(GLOB_RECURSE headers LIST_DIRECTORIES false ${PROJECT_SOURCE_DIR}/include/sqlpp23/${ARG_HEADER_DIR}/*.h)
    string(TOLOWER ${ARG_NAME} lc_name)
    _add_build_set_if(NO_INSTALL ARG_NO_INSTALL)
    _add_build_common(
        CONFIG_SCRIPT Sqlpp23${ARG_NAME}Config.cmake
        DEFINES ${ARG_DEFINES}
        DEPENDENCIES sqlpp23::core ${ARG_DEPENDENCIES}
        HEADERS ${headers}
        MODULE_INTERFACE ${ARG_MODULE_INTERFACE}
        PACKAGE ${ARG_PACKAGE}
        TARGET_NAME sqlpp23_${lc_name}
        TARGET_ALIAS sqlpp23::${lc_name}
        TARGET_EXPORTED ${lc_name}
        TARGET_FILE Sqlpp23${ARG_NAME}Targets
        ${NO_INSTALL}
    )
endfunction()

function(_add_build_common)
    set(options NO_INSTALL)
    set(oneValueArgs CONFIG_SCRIPT MODULE_INTERFACE PACKAGE TARGET_NAME TARGET_ALIAS TARGET_EXPORTED TARGET_FILE)
    set(multiValueArgs DEFINES DEPENDENCIES HEADERS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

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
    target_link_libraries(${ARG_TARGET_NAME} INTERFACE ${ARG_DEPENDENCIES})

    # Add the component headers to the HEADERS file set. This also adds the base directory to the
    # target's build interface include directories.
    target_sources(
        ${ARG_TARGET_NAME}
        INTERFACE
        FILE_SET HEADERS
            BASE_DIRS ${PROJECT_SOURCE_DIR}/include
            FILES ${ARG_HEADERS}
    )
    if(NOT ARG_NO_INSTALL)
        # Install the config script
        install(
            FILES ${PROJECT_SOURCE_DIR}/cmake/configs/${ARG_CONFIG_SCRIPT}
            DESTINATION ${SQLPP23_INSTALL_CMAKEDIR}
        )
        # Install the component output artifacts
        install(
            TARGETS ${ARG_TARGET_NAME}
            EXPORT ${ARG_TARGET_FILE}
            FILE_SET HEADERS
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
	# Install the config targets file
        install(
            EXPORT ${ARG_TARGET_FILE}
            DESTINATION ${SQLPP23_INSTALL_CMAKEDIR}
            NAMESPACE sqlpp23::
        )
        if(BUILD_WITH_MODULES AND ARG_MODULE_INTERFACE)
            # Install the component's module interface file
            install(
                FILES ${PROJECT_SOURCE_DIR}/modules/${ARG_MODULE_INTERFACE}
                DESTINATION ${CMAKE_INSTALL_PREFIX}/modules/sqlpp23
            )
        endif()
        if(ARG_PACKAGE)
            # If the package needs a special find script, copy it to the destination scripts directory
            set(find_script ${PROJECT_SOURCE_DIR}/cmake/modules/Find${ARG_PACKAGE}.cmake)
            if(EXISTS ${find_script})
                install(FILES ${find_script} DESTINATION ${SQLPP23_INSTALL_CMAKEDIR})
            endif()
        endif()
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
