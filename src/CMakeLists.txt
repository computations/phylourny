add_library(phylourny_lib
    STATIC
    tournament.cpp
    model.cpp
    sampler.cpp
    summary.cpp
    match.cpp
    cli.cpp
    util.cpp
    tournament_node.cpp
    tournament_factory.cpp
    single_node.cpp
    simulation_node.cpp
    mcmc.cpp
    program_options.cpp
)

add_executable(phylourny
    main.cpp
)

find_package(OpenMP)

if(OpenMP_CXX_FOUND)
  target_link_libraries(phylourny_lib pthread $<$<CONFIG:Release>:OpenMP::OpenMP_CXX>)
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
    -fno-omit-frame-pointer -O3)
target_compile_definitions(phylourny PRIVATE
    GIT_REV=${BUILD_VERSION}
    GIT_COMMIT=${BUILD_COMMIT}
    BUILD_DATE=${BUILD_DATE}
    BUILD_DDATE=${BUILD_DDATE}
  )
if(PHYLOURNY_JOKE_BUILD)
target_compile_definitions(phylourny PRIVATE
  JOKE_BUILD=true
    )
  endif()
