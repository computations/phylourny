add_library(phylourny_lib
    STATIC
    tournament.cpp
    dataset.cpp
    sampler.cpp
    summary.cpp
    match.cpp
    cli.cpp
    util.cpp
    tournament_node.cpp
    tournament_factory.cpp
    single_node.cpp
    simulation_node.cpp
)

add_executable(phylourny
    main.cpp
)

find_package(OpenMP)

target_link_libraries(phylourny_lib pthread $<$<CONFIG:Release>:OpenMP::OpenMP_CXX>)

set_target_properties(phylourny_lib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

target_compile_options(phylourny_lib PRIVATE -Wall -Wextra -Wshadow
    -Wdouble-promotion -Wmissing-include-dirs -pedantic -Wsign-conversion)

set_target_properties(phylourny PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    INTERPROCEDURAL_OPTIMIZATION TRUE
)

target_link_libraries(phylourny ${LINK_LIBS} phylourny_lib pthread $<$<CONFIG:Release>:OpenMP::OpenMP_CXX>)
target_compile_options(phylourny PRIVATE -Wall -Wextra -Wshadow
    -Wdouble-promotion -Wmissing-include-dirs -pedantic -Wsign-conversion)
target_compile_definitions(phylourny PRIVATE
    GIT_REV=${BUILD_VERSION}
    GIT_COMMIT=${BUILD_COMMIT}
    BUILD_DATE=${BUILD_DATE})
