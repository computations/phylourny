cmake_minimum_required(VERSION 3.9)
project(phylourny)

find_package(Git)
if(GIT_FOUND)
    if(EXISTS ${CMAKE_SOURCE_DIR})
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags
            WORKING_DIRECTORY "${local_dir}"
            OUTPUT_VARIABLE BUILD_VERSION
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        if (${RD_BUILD_VERSION})
            string(REGEX REPLACE "-g.+" "" BUILD_VERSION ${RD_BUILD_VERSION})
            if(STATIC_BUILD)
                set(BUILD_VERSION "${BUILD_VERSION}-static")
            endif()
        endif()
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
            WORKING_DIRECTORY "${local_dir}"
            OUTPUT_VARIABLE BUILD_COMMIT
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()
endif()

string(TIMESTAMP BUILD_DATE "%Y-%m-%d %H:%M:%S")

set(CMAKE_CXX_FLAGS_RELEASE "-fno-omit-frame-pointer" CACHE INTERNAL "")
set(CMAKE_C_FLAGS_RELEASE "-fno-omit-frame-pointer" CACHE INTERNAL "")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
include_directories(lib)
include_directories(lib/json/single_include/nlohmann)
include_directories(SYSTEM lib/csv)
add_subdirectory(src)

find_library(ASAN_FOUND asan)
if(ASAN_FOUND)
    add_subdirectory(test)
endif()

find_package(benchmark REQUIRED)
add_subdirectory(bench)
