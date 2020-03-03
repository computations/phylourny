set(CMAKE_BUILD_TYPE Debug)
add_executable(phylourny_test
    main.cpp
    tournament.cpp
)

find_package (Eigen3 3.3 REQUIRED NO_MODULE)

set(TEST_LINK_LIBS Catch2::Catch2 asan Eigen3::Eigen)
target_compile_options(phylourny_test PRIVATE -Wall -Wextra -pedantic
    -fno-omit-frame-pointer
    -fsanitize=address)
target_include_directories(phylourny_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(phylourny_test PRIVATE
    ${CMAKE_SOURCE_DIR}/test/lib/)
target_link_libraries(phylourny_test ${TEST_LINK_LIBS})
