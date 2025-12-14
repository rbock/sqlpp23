# Copyright (c) 2013-2016, Roland Bock, Alexey Elymanov
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

include(FindPython3)

if (${Python3_Interpreter_FOUND})
    execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "import pyparsing"
        RESULT_VARIABLE PythonRESULT
        OUTPUT_VARIABLE PythonOUTPUT
        ERROR_VARIABLE PythonERROR
    )

    if (${PythonRESULT})
        message(WARNING "Pyparsing is not installed. Disabling sqlpp23-ddl2cpp tests")
    else()
        message(STATUS "Pyparsing is installed: Enabling sqlpp23-ddl2cpp tests.")

        cmake_path(SET ddl2cpp NORMALIZE "${PROJECT_SOURCE_DIR}/scripts/sqlpp23-ddl2cpp")

        add_test(NAME sqlpp23.scripts.ddl2cpp.parser
            COMMAND "${Python3_EXECUTABLE}" "${ddl2cpp}"
                    "--self-test"
                    )

        add_test(NAME sqlpp23.scripts.ddl2cpp.bad_will_fail
            COMMAND "${Python3_EXECUTABLE}" "${ddl2cpp}"
                    "--path-to-ddl" "${CMAKE_CURRENT_LIST_DIR}/ddl2cpp_sample_bad.sql"
                    "--path-to-header" "${CMAKE_CURRENT_BINARY_DIR}/fail.h"
                    "--namespace" "test")
        set_tests_properties(sqlpp23.scripts.ddl2cpp.bad_will_fail PROPERTIES WILL_FAIL 1)

        add_test(NAME sqlpp23.scripts.ddl2cpp.bad_has_parse_error
            COMMAND "${Python3_EXECUTABLE}" "${ddl2cpp}"
                    "--path-to-ddl" "${CMAKE_CURRENT_LIST_DIR}/ddl2cpp_sample_bad.sql"
                    "--path-to-header" "${CMAKE_CURRENT_BINARY_DIR}/fail.h"
                    "--namespace" "test")
        set_tests_properties(sqlpp23.scripts.ddl2cpp.bad_has_parse_error PROPERTIES
          PASS_REGULAR_EXPRESSION "ERROR: Failed to parse.*")

        add_test(NAME sqlpp23.scripts.ddl2cpp.good_succeeds
            COMMAND "${Python3_EXECUTABLE}" "${ddl2cpp}"
                    "--path-to-ddl" "${CMAKE_CURRENT_LIST_DIR}/ddl2cpp_sample_good.sql"
                    "--path-to-header" "${CMAKE_CURRENT_BINARY_DIR}/good.h"
                    "--namespace" "test")

        include_directories(${CMAKE_CURRENT_BINARY_DIR})

        foreach(sample_name sample sample_identity_naming)
            set(sqlpp.scripts.generated.sample.include "${CMAKE_CURRENT_BINARY_DIR}/${sample_name}.h")
            if(sample_name STREQUAL "sample_identity_naming")
                set(naming_style identity)
            else()
                set(naming_style camel-case)
            endif()
            add_custom_command(
                OUTPUT "${sqlpp.scripts.generated.sample.include}"
                COMMAND "${Python3_EXECUTABLE}" "${ddl2cpp}"
                        "--path-to-ddl" "${CMAKE_CURRENT_LIST_DIR}/ddl2cpp_sample_good.sql"
                        "--path-to-header" "${sqlpp.scripts.generated.sample.include}"
                        "--naming-style" "${naming_style}"
                        "--namespace" "test"
                        "--suppress-timestamp-warning"
                DEPENDS "${CMAKE_CURRENT_LIST_DIR}/ddl2cpp_sample_good.sql"
                        "${ddl2cpp}"
                VERBATIM)

            add_executable(sqlpp.scripts.compiled.${sample_name} ${sample_name}.cpp
                "${sqlpp.scripts.generated.sample.include}")
            target_link_libraries(sqlpp.scripts.compiled.${sample_name} PRIVATE sqlpp23::core)
        endforeach()

        # Invalid .types names
        foreach(bad_type "booltype" "invalid" "serial5" "typeint")
            set(bad_type_test_name "sqlpp23.scripts.ddl2cpp.bad_type.${bad_type}")
            add_test(NAME "${bad_type_test_name}"
                COMMAND "${Python3_EXECUTABLE}" "${ddl2cpp}"
                        "--path-to-ddl" "${CMAKE_CURRENT_LIST_DIR}/ddl2cpp_sample_bad_type_${bad_type}.sql"
                        "--path-to-header" "${CMAKE_CURRENT_BINARY_DIR}/fail.h"
                        "--namespace" "test"
                        "--suppress-timestamp-warning")
            set_tests_properties("${bad_type_test_name}" PROPERTIES
                PASS_REGULAR_EXPRESSION "ERROR: Unsupported SQL data type\\(s\\).")
        endforeach()

        # Custom types defined in a CSV file
        set(custom_type_stem "ddl2cpp_sample_good_custom_type")
        set(custom_type_sql "${CMAKE_CURRENT_LIST_DIR}/${custom_type_stem}.sql")
        set(custom_type_cpp "${custom_type_stem}.cpp")
        set(custom_type_exe "sqlpp.scripts.compiled.${custom_type_stem}")
        set(custom_type_headers "")
        foreach(custom_type_suffix "old" "new")
            set(custom_type_csv "${CMAKE_CURRENT_LIST_DIR}/${custom_type_stem}_${custom_type_suffix}.csv")
            set(custom_type_header "${CMAKE_CURRENT_BINARY_DIR}/${custom_type_stem}_${custom_type_suffix}.h")
            list(APPEND custom_type_headers "${custom_type_header}")
            add_custom_command(
                OUTPUT "${custom_type_header}"
                COMMAND "${Python3_EXECUTABLE}" "${ddl2cpp}"
                        "--path-to-custom-types=${custom_type_csv}"
                        "--path-to-ddl" "${custom_type_sql}"
                        "--path-to-header" "${custom_type_header}"
                        "--namespace" "test::dbm_${custom_type_suffix}"
                        "--suppress-timestamp-warning"
                DEPENDS "${custom_type_sql}"
                        "${custom_type_csv}"
                        "${ddl2cpp}"
                VERBATIM)
        endforeach()
        add_executable("${custom_type_exe}" "${custom_type_cpp}" ${custom_type_headers})
        target_link_libraries("${custom_type_exe}" PRIVATE sqlpp23::core)
    endif()
endif()
