cmake_minimum_required(VERSION 3.00)
project(root_digger)

find_package(Git)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags
        WORKING_DIRECTORY "${local_dir}"
        OUTPUT_VARIABLE BUILD_VERSION
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX REPLACE "-g.+" "" BUILD_VERSION ${BUILD_VERSION})
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        WORKING_DIRECTORY "${local_dir}"
        OUTPUT_VARIABLE BUILD_COMMIT
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

string(TIMESTAMP BUILD_DATE "%Y-%m-%d %H:%M:%S")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
add_subdirectory(src)

find_library(ASAN_FOUND asan)
if(ASAN_FOUND)
    add_subdirectory(test)
endif()
