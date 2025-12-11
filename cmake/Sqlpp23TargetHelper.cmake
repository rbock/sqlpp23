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

function(add_component)
    set(options)
    set(oneValueArgs NAME PACKAGE)
    set(multiValueArgs DEPENDENCIES DEFINES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(ARG_NAME)
        set(TARGET_NAME sqlpp23_${ARG_NAME})
        set(TARGET_ALIAS sqlpp23::${ARG_NAME})
        set(TARGET_EXPORTED ${ARG_NAME})
    else()
        set(TARGET_NAME sqlpp23)
        set(TARGET_ALIAS sqlpp23::sqlpp23)
        set(TARGET_EXPORTED sqlpp23)
    endif()
    if(ARG_PACKAGE AND DEPENDENCY_CHECK)
        find_package(${ARG_PACKAGE} REQUIRED)
    endif()
    add_library(${TARGET_NAME} INTERFACE)
    add_library(${TARGET_ALIAS} ALIAS ${TARGET_NAME})
    set_target_properties(${TARGET_NAME} PROPERTIES EXPORT_NAME ${TARGET_EXPORTED})
    target_include_directories(${TARGET_NAME} INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
    target_compile_features(${TARGET_NAME} INTERFACE cxx_std_23)
    if(ARG_DEFINES)
        target_compile_definitions(${TARGET_NAME} INTERFACE ${ARG_DEFINES})
    endif()
    if(ARG_DEPENDENCIES)
        target_link_libraries(${TARGET_NAME} INTERFACE sqlpp23 ${ARG_DEPENDENCIES})
    endif()
endfunction()

function(install_component)
    set(options)
    set(oneValueArgs HEADER_DIR NAME)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(TARGET sqlpp23)
    if(ARG_NAME)
        string(TOLOWER ${ARG_NAME} NAME_LC)
        string(APPEND TARGET "_" ${NAME_LC})
    endif()

    install(FILES ${PROJECT_SOURCE_DIR}/cmake/configs/Sqlpp23${ARG_NAME}Config.cmake
        DESTINATION ${SQLPP23_INSTALL_CMAKEDIR}
    )

    install(TARGETS ${TARGET}
        EXPORT Sqlpp23Targets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/sqlpp23/${ARG_HEADER_DIR}
        DESTINATION include/sqlpp23
        FILES_MATCHING
        PATTERN *.h
    )

    set(FIND_SCRIPT ${PROJECT_SOURCE_DIR}/cmake/modules/Find{ARG_NAME}.cmake)
    if(EXISTS ${FIND_SCRIPT})
        install(FILES ${FIND_SCRIPT} DESTINATION ${SQLPP23_INSTALL_CMAKEDIR})
    endif()
endfunction()
