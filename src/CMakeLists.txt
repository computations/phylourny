add_executable(phylourny
    main.cpp
)

set_target_properties(phylourny PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

target_link_libraries(phylourny ${LINK_LIBS})
target_compile_options(phylourny PRIVATE -Wall -Wextra -Wshadow
    -Wdouble-promotion -Wmissing-include-dirs -Wtrampolines
    -Wmisleading-indentation -pedantic)
target_compile_definitions(phylourny PRIVATE
    GIT_REV=${BUILD_VERSION}
    GIT_COMMIT=${BUILD_COMMIT}
    BUILD_DATE=${BUILD_DATE})
