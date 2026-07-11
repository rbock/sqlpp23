# Copyright (c) 2026, Vesselin Atanasov
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

function(add_testing_base)
    add_library(sqlpp23_testing INTERFACE)
    target_include_directories(sqlpp23_testing INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    if(BUILD_WITH_MODULES)
        target_link_libraries(sqlpp23_testing INTERFACE sqlpp23::core_module)
        target_compile_definitions(sqlpp23_testing INTERFACE BUILD_WITH_MODULES)
    else()
        target_link_libraries(sqlpp23_testing INTERFACE sqlpp23::core)
    endif()
    if (NOT MSVC)
        target_compile_options(sqlpp23_testing INTERFACE -Wall -Wextra -pedantic -Wshadow -Wconversion)
    endif ()
endfunction()

function(add_testing_target)
    set(options)
    set(oneValueArgs NAME)
    set(multiValueArgs DEFINES MOD_DEPS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(BUILD_WITH_MODULES)
        # We are building with modules, so the _testing target created by this function MUST be a regular
        # (non-INTERFACE) library. Otherwise, the object files owned by the OBJECT library dependencies
        # in MOD_DEPS won't be linked, and we are going to get missing symbols during the linking (namely
        # the static module initializer function won't be found).
        #
        # For details on how OBJECT libraries are linked see
        # https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#object-libraries
        # https://cmake.org/cmake/help/latest/command/target_link_libraries.html#linking-object-libraries
        set(lib_type)
        set(lib_prop_scope PUBLIC)
    else()
        # We are building without modules and the _testing target created by this function won't have any
        # compiled files, and CMake will not let us build a empty regular (non-INTERFACE) library. So in
        # this case we have to use an INTERFACE library.
        set(lib_type INTERFACE)
        set(lib_prop_scope INTERFACE)
    endif()
    set(target_name "sqlpp23_${ARG_NAME}_testing")
    add_library(${target_name} ${lib_type})
    if(ARG_DEFINES)
        target_compile_definitions(${target_name} ${lib_prop_scope} ${ARG_DEFINES})
    endif()
    target_link_libraries(${target_name} ${lib_prop_scope} sqlpp23_testing)
    if(BUILD_WITH_MODULES)
        set(mod_file "${PROJECT_SOURCE_DIR}/tests/${ARG_NAME}/modules/sqlpp23.test.${ARG_NAME}.tables.cppm")
        target_sources(${target_name} PUBLIC FILE_SET CXX_MODULES FILES "${mod_file}")
        target_link_libraries(${target_name} PUBLIC sqlpp23::core_module ${ARG_MOD_DEPS})
    endif()
endfunction()

function(create_tests_assert)
    _create_tests_get_component_and_group(component group)
    math(EXPR rem "${ARGC}%2")
    if(rem)
        message(FATAL_ERROR "Number of arguments must be even")
    endif()
    while(ARGV)
        list(POP_FRONT ARGV name pattern)
        _create_tests_assert_item(${component} ${group} ${name} "${pattern}")
    endwhile()
endfunction()

function(_create_tests_assert_item component group name pattern)
    _create_tests_assert_setup(${component} ${group} ${name})
    _create_tests_assert_main(${component} ${group} ${name} "${pattern}")
endfunction()

function(_create_tests_assert_setup component group name)
    _create_tests_get_name_target(target ${component} ${group} setup ${name})
    _create_tests_add_exe_target(
        COMPONENT ${component}
        SOURCES ${name}.cpp
        TARGET ${target}
    )
    _create_tests_get_name_test(test ${component} ${group} setup ${name})
    add_test(NAME ${test} COMMAND ${target})
endfunction()

function(_create_tests_assert_main component group name pattern)
    _create_tests_get_name_target(target ${component} ${group} ${name})
    _create_tests_add_exe_target(
        COMPONENT ${component}
        DEFINES SQLPP_CHECK_STATIC_ASSERT
        EXCLUDE_FROM_ALL
        SOURCES ${name}.cpp
        TARGET ${target}
    )
    _create_tests_get_name_test(test ${component} ${group} ${name})
    add_test(NAME ${test}
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${target}
    )
    set_property(TEST ${test} PROPERTY PASS_REGULAR_EXPRESSION "${pattern}")
endfunction()

function(create_tests_combined)
    _create_tests_get_component_and_group(component group)
    _create_tests_get_name_target(target ${component} ${group} "combined")
    set(test_files)
    foreach(name IN LISTS ARGV)
        list(APPEND test_files "${name}.cpp")
    endforeach()
    create_test_sourcelist(all_sources test_main.cpp ${test_files})
    _create_tests_add_exe_target(
        COMPONENT ${component}
        SOURCES ${all_sources}
        TARGET ${target}
    )
    foreach(name IN LISTS ARGV)
        _create_tests_get_name_test(test ${component} ${group} ${name})
        add_test(NAME ${test} COMMAND ${target} ${name})
    endforeach()
endfunction()

function(create_tests_compiles)
    _create_tests_get_component_and_group(component group)
    foreach(name ${ARGV})
        _create_tests_compiles_item(${component} ${group} ${name})
    endforeach()
endfunction()

function(_create_tests_compiles_item component group name)
    _create_tests_get_name_target(target ${component} ${group} ${name})
    _create_tests_add_exe_target(
        COMPONENT ${component}
        SOURCES ${name}.cpp
        TARGET ${target}
    )
endfunction()

function(create_tests_group)
    _create_tests_get_component_and_group(component group)
    foreach(name ${ARGV})
        _create_tests_group_item(${component} ${group} ${name})
    endforeach()
endfunction()

function(_create_tests_group_item component group name)
    _create_tests_get_name_target(target ${component} ${group} ${name})
    _create_tests_add_exe_target(
        COMPONENT ${component}
        SOURCES ${name}.cpp
        TARGET ${target}
    )
    _create_tests_get_name_test(test ${component} ${group} ${name})
    add_test(NAME ${test} COMMAND ${target})
endfunction()

function(_create_tests_get_component_and_group component_var group_var)
    cmake_path(APPEND PROJECT_SOURCE_DIR "tests" OUTPUT_VARIABLE tests_dir)
    cmake_path(
        RELATIVE_PATH
        CMAKE_CURRENT_SOURCE_DIR
        BASE_DIRECTORY "${tests_dir}"
        OUTPUT_VARIABLE relative_path
    )
    string(REPLACE "/" ";" relative_dirs "${relative_path}")
    list(POP_FRONT relative_dirs component)
    list(JOIN relative_dirs "_" group)
    set("${component_var}" "${component}" PARENT_SCOPE)
    set("${group_var}" "${group}" PARENT_SCOPE)
endfunction()

macro(_create_tests_get_name_target name_var)
    _create_tests_get_name_common(${name_var} "_" ${ARGN})
endmacro()

macro(_create_tests_get_name_test name_var)
    _create_tests_get_name_common(${name_var} "." ${ARGN})
endmacro()

macro(_create_tests_get_name_common name_var delimiter)
    string(JOIN ${delimiter} combined sqlpp23 ${ARGN})
    set("${name_var}" "${combined}")
endmacro()

function(_create_tests_add_exe_target)
    set(options EXCLUDE_FROM_ALL)
    set(oneValueArgs COMPONENT TARGET)
    set(multiValueArgs DEFINES SOURCES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_executable(${ARG_TARGET} ${EXCLUDE_FROM_ALL} ${ARG_SOURCES})
    if(ARG_EXCLUDE_FROM_ALL)
        set_property(TARGET ${ARG_TARGET} PROPERTY EXCLUDE_FROM_ALL TRUE)
    endif()
    if(ARG_DEFINES)
        target_compile_definitions(${ARG_TARGET} PRIVATE ${ARG_DEFINES})
    endif()
    if((${ARG_COMPONENT} STREQUAL sqlite3) AND BUILD_SQLCIPHER_CONNECTOR)
        set(dep sqlcipher)
    else()
        set(dep ${ARG_COMPONENT})
    endif()
    target_link_libraries(${ARG_TARGET} PRIVATE sqlpp23::${dep} sqlpp23_${ARG_COMPONENT}_testing)
endfunction()
