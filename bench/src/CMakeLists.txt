add_executable(phylourny_bench
    ${RD_SOURCES}
    main.cpp
    tournament.cpp
)

set_target_properties(phylourny_bench PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

set(TEST_LINK_LIBS benchmark)
target_compile_options(phylourny_bench PRIVATE -Wall -Wextra -pedantic)
target_include_directories(phylourny_bench PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(phylourny_bench phylourny_lib ${TEST_LINK_LIBS})
