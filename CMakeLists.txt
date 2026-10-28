cmake_minimum_required(VERSION 3.22)
project(dmalloc VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wconversion
    -Wsign-conversion
    -march=native
)

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto=auto -DNDEBUG")

# ------------------------------------------------------------------------------
# Core Header-Only Library
# ------------------------------------------------------------------------------
add_library(dmalloc_core INTERFACE)
target_include_directories(dmalloc_core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ------------------------------------------------------------------------------
# Main Executable
# ------------------------------------------------------------------------------
add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE dmalloc_core)

# ------------------------------------------------------------------------------
# Tests (GoogleTest)
# ------------------------------------------------------------------------------
enable_testing()
find_package(GTest REQUIRED)

# Dynamically gather all .cpp files in tests/ and watch for new ones
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "tests/*.cpp")

# Create target, but force the output executable to literally just be "test"
add_executable(test_bin ${TEST_SOURCES})
set_target_properties(test_bin PROPERTIES OUTPUT_NAME "run_tests")

target_link_libraries(test_bin PRIVATE dmalloc_core GTest::gtest GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_bin)

# ------------------------------------------------------------------------------
# Benchmarks (Google Benchmark)
# ------------------------------------------------------------------------------
find_package(benchmark REQUIRED)

# Dynamically gather all .cpp files in benchmarks/ and watch for new ones
file(GLOB_RECURSE BENCH_SOURCES CONFIGURE_DEPENDS "benchmarks/*.cpp")

# Create target, force output executable to literally just be "benchmark"
add_executable(bench_bin ${BENCH_SOURCES})
set_target_properties(bench_bin PROPERTIES OUTPUT_NAME "run_benchmarks")

target_link_libraries(bench_bin PRIVATE dmalloc_core benchmark::benchmark benchmark::benchmark_main)
