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
)

add_executable(phylourny
    main.cpp
)

set_target_properties(phylourny_lib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

set_target_properties(phylourny PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    INTERPROCEDURAL_OPTIMIZATION TRUE
)

target_link_libraries(phylourny ${LINK_LIBS} phylourny_lib pthread)
target_compile_options(phylourny PRIVATE -Wall -Wextra -Wshadow
    -Wdouble-promotion -Wmissing-include-dirs -pedantic)
target_compile_definitions(phylourny PRIVATE
    GIT_REV=${BUILD_VERSION}
    GIT_COMMIT=${BUILD_COMMIT}
    BUILD_DATE=${BUILD_DATE})
