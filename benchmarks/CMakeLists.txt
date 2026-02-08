include(FetchContent)

# Fetch Google Benchmark
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
    GIT_SHALLOW TRUE
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(benchmark)

# Benchmark executables
add_executable(yapl_benchmarks
    blocking_queue_bench.cpp
    media_clock_bench.cpp
    memory_allocation_bench.cpp
)

target_link_libraries(yapl_benchmarks
    PRIVATE
        yapl::yapl
        benchmark::benchmark
        benchmark::benchmark_main
)

target_compile_options(yapl_benchmarks PRIVATE -O3)
