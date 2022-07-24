set(CMAKE_BUILD_TYPE Debug)
add_executable(phylourny_test
    main.cpp
    util.cpp
    tournament.cpp
    dataset.cpp
    sampler.cpp
    single.cpp
    simulation.cpp
)

set_target_properties(phylourny_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

find_package(OpenMP)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)

set(TEST_LINK_LIBS Catch2::Catch2WithMain asan)
target_compile_options(phylourny_test PRIVATE -Wall -Wextra -pedantic
    -fno-omit-frame-pointer
    -fsanitize=address)
target_include_directories(phylourny_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(phylourny_test PRIVATE
    ${CMAKE_SOURCE_DIR}/test/lib/)
target_link_libraries(phylourny_test phylourny_lib ${TEST_LINK_LIBS} OpenMP::OpenMP_CXX)
