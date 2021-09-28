cmake_minimum_required(VERSION 3.9)
project(phylourny)

option(PHYLOURNY_BUILD_TESTS "Build tests" ON)
option(PHYLOURNY_BUILD_BENCH "Build benchmarks" ON)

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

option(PHYLOURNY_BUILD_DOCS "Build documentation" ON)

# check if Doxygen is installed
if(PHYLOURNY_BUILD_DOCS)
  # Code taken from https://vicrucann.github.io/tutorials/quick-cmake-doxygen/
  find_package(Doxygen)
  if (DOXYGEN_FOUND)
      # set input and output files
      set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
      set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

      # request to configure the file
      configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
      message("Doxygen build started")

      # note the option ALL which allows to build the docs together with the application
      add_custom_target(doc_doxygen ALL
          COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
          COMMENT "Generating API documentation with Doxygen"
          VERBATIM )
  else (DOXYGEN_FOUND)
    message("Doxygen need to be installed to generate the doxygen documentation")
  endif (DOXYGEN_FOUND)
endif(PHYLOURNY_BUILD_DOCS)


string(TIMESTAMP BUILD_DATE "%Y-%m-%d %H:%M:%S")

set(CMAKE_CXX_FLAGS_RELEASE "-fno-omit-frame-pointer" CACHE INTERNAL "")
set(CMAKE_C_FLAGS_RELEASE "-fno-omit-frame-pointer" CACHE INTERNAL "")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
add_subdirectory(src)
add_subdirectory(lib)

find_library(ASAN_FOUND asan)
if(ASAN_FOUND AND PHYLOURNY_BUILD_TESTS)
    add_subdirectory(test)
endif()

if(PHYLOURNY_BUILD_BENCH)
  add_subdirectory(bench)
endif()
