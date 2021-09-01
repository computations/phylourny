find_package(benchmark QUIET)
if(benchmark_FOUND)
    add_subdirectory(src)
endif()
