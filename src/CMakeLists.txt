add_library(phylourny_lib
    STATIC
    tournament.cpp
    model.cpp
    summary.cpp
    match.cpp
    util.cpp
    tournament_node.cpp
    tournament_factory.cpp
    single_node.cpp
    mcmc.cpp
    program_options.cpp
)

target_include_directories(phylourny_lib SYSTEM PUBLIC ${CMAKE_SOURCE_DIR}/lib/csv
  ${CMAKE_SOURCE_DIR}/lib/json/single_include/nlohmann
  )

add_executable(phylourny
    main.cpp
)

find_package(OpenMP)

target_link_libraries(phylourny_lib PUBLIC sul::dynamic_bitset)
if(OpenMP_CXX_FOUND)
  target_link_libraries(phylourny_lib PUBLIC pthread $<$<CONFIG:Release>:OpenMP::OpenMP_CXX>)
endif()

set_target_properties(phylourny_lib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    INTERPROCEDURAL_OPTIMIZATION TRUE
)

target_compile_options(phylourny_lib PRIVATE -Wall -Wextra -Wshadow
    -Wdouble-promotion -Wmissing-include-dirs -pedantic -Wsign-conversion
    -fno-omit-frame-pointer) 

set_target_properties(phylourny PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    INTERPROCEDURAL_OPTIMIZATION TRUE
)

option(PHYLOURNY_JOKE_BUILD ON "Turn on jokes")
option(PHYLOURNY_TIME_EVALS OFF "Enable timing of the tournament evaluations")

find_program(DDATE "ddate")

if(DDATE)
  exec_program("ddate +'%{%A, the %e of %B%}, %Y.'"
    OUTPUT_VARIABLE BUILD_DDATE)
endif()

target_link_libraries(phylourny ${LINK_LIBS} phylourny_lib)

if(NOT OpenMP_CXX_FOUND)
  target_link_libraries(phylourny ${LINK_LIBS} -static)
endif()

target_compile_options(phylourny PRIVATE -Wall -Wextra -Wshadow
    -Wdouble-promotion -Wmissing-include-dirs -pedantic -Wsign-conversion
    -fno-omit-frame-pointer)
target_compile_definitions(phylourny PRIVATE
    GIT_REV=${BUILD_VERSION}
    GIT_COMMIT=${BUILD_COMMIT}
    BUILD_DATE=${BUILD_DATE}
    BUILD_DDATE=${BUILD_DDATE}
  )

if(PHYLOURNY_JOKE_BUILD)
  target_compile_definitions(phylourny_lib PRIVATE JOKE_BUILD=true)
  target_compile_definitions(phylourny PRIVATE JOKE_BUILD=true)
endif()

if(PHYLOURNY_TIME_EVALS)
  target_compile_definitions(phylourny_lib PRIVATE PHYLOURNY_TIME_EVALS)
  target_compile_definitions(phylourny PRIVATE PHYLOURNY_TIME_EVALS)
endif()
