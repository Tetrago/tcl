cmake_minimum_required(VERSION 3.10)
project(tcl LANGUAGES CXX)

option(TCL_BUILD_TESTS "Build unit tests" ON)
option(TCL_ENABLE_COVERAGE "Enable coverage reporting" OFF)
option(TCL_ENABLE_ASAN "Enable address sanitization" OFF)

set(HEADERS "forward_list.hpp")

list(TRANSFORM HEADERS PREPEND "include/tcl/")

add_library(${PROJECT_NAME} INTERFACE ${HEADERS})

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

target_include_directories(${PROJECT_NAME} INTERFACE "include")

if(TCL_BUILD_TESTS)
    include(CTest)
    enable_testing()

    include(FetchContent)

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.14.0
    )

    FetchContent_MakeAvailable(Catch2)

    set(TEST_SOURCES "forward_list.cpp")

    list(TRANSFORM TEST_SOURCES PREPEND "tests/")

    add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})

    target_link_libraries(
        ${PROJECT_NAME}_tests
        PRIVATE ${PROJECT_NAME} Catch2::Catch2WithMain
    )

    if(TCL_ENABLE_COVERAGE)
        find_program(LLVM_PROFDATA llvm-profdata)
        find_program(LLVM_COV llvm-cov)

        set(COVERAGE_DIR "${PROJECT_BINARY_DIR}/coverage")
        set(PROFRAW_FILE "${COVERAGE_DIR}/default.profraw")
        set(PROFDATA_FILE "${COVERAGE_DIR}/coverage.profdata")

        if(LLVM_PROFDATA AND LLVM_COV AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            add_executable(${PROJECT_NAME}_tests_cov ${TEST_SOURCES})

            target_link_libraries(
                ${PROJECT_NAME}_tests_cov
                PRIVATE ${PROJECT_NAME} Catch2::Catch2WithMain
            )

            target_link_options(
                ${PROJECT_NAME}_tests_cov
                PRIVATE -fprofile-instr-generate
            )

            target_compile_options(
                ${PROJECT_NAME}_tests_cov
                PRIVATE -O0 -g -fprofile-instr-generate -fcoverage-mapping
            )

            add_custom_target(
                ${PROJECT_NAME}_tests_cov_report
                COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_DIR}
                COMMAND
                    LLVM_PROFILE_FILE=${PROFRAW_FILE}
                    $<TARGET_FILE:${PROJECT_NAME}_tests_cov>
                COMMAND
                    ${LLVM_PROFDATA} merge -sparse ${PROFRAW_FILE} -o
                    ${PROFDATA_FILE}
                COMMAND
                    ${LLVM_COV} show $<TARGET_FILE:${PROJECT_NAME}_tests_cov>
                    -instr-profile=${PROFDATA_FILE} -format=html
                    -output-dir=${COVERAGE_DIR}/html
                    -show-line-counts-or-regions -show-instantiations
                    -ignore-filename-regex=".*/_deps/.*"
                COMMAND
                    ${LLVM_COV} report $<TARGET_FILE:${PROJECT_NAME}_tests_cov>
                    -instr-profile=${PROFDATA_FILE}
                    -ignore-filename-regex=".*/_deps/.*"
                DEPENDS ${PROJECT_NAME}_tests
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                COMMENT "Generating LLVM coverage report..."
            )
        else()
            message(WARNING "Coverage reporting is enabled, but not available")
        endif()
    endif()

    if(TCL_ENABLE_ASAN)
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            add_executable(${PROJECT_NAME}_tests_asan ${TEST_SOURCES})

            target_link_libraries(
                ${PROJECT_NAME}_tests_asan
                PRIVATE ${PROJECT_NAME} Catch2::Catch2WithMain
            )

            target_link_options(
                ${PROJECT_NAME}_tests_asan
                PRIVATE -fsanitize=address -fno-omit-frame-pointer
            )

            target_compile_options(
                ${PROJECT_NAME}_tests_asan
                PRIVATE -O1 -fsanitize=address -fno-omit-frame-pointer
            )
        else()
            message(
                WARNING
                "Address sanitization is enabled, but not available"
            )
        endif()
    endif()

    include(Catch)
    catch_discover_tests(${PROJECT_NAME}_tests)
endif()
