# Standalone comparison: Boost is required only when building this benchmark.
cmake_minimum_required(VERSION 3.16)
project(robometry_buffer_benchmark LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(Boost REQUIRED)
find_package(matioCpp 0.2.0 REQUIRED)

# Allows the same harness to measure a saved version of the headers.
set(ROBOMETRY_BENCHMARK_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../src/librobometry/include"
    CACHE PATH "Robometry headers to benchmark")

foreach(target circular_buffer_benchmark circular_buffer_allocations)
  add_executable(${target} CircularBufferBenchmark.cpp)
  target_include_directories(${target} PRIVATE "${ROBOMETRY_BENCHMARK_INCLUDE_DIR}")
  target_link_libraries(${target} PRIVATE Boost::boost matioCpp::matioCpp)
  target_compile_definitions(${target} PRIVATE
    ROBOMETRY_BENCHMARK_COMPILER="${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}")
endforeach()

# Allocation instrumentation is deliberately absent from the timing executable.
target_compile_definitions(circular_buffer_allocations PRIVATE ROBOMETRY_ALLOCATION_PROBE)

include(CTest)
if(BUILD_TESTING)
  add_test(NAME capacity_allocations
           COMMAND circular_buffer_allocations --capacity)
  # This runs the payload/order checks, not a performance threshold test.
  add_test(NAME capacity_preservation_smoke
           COMMAND circular_buffer_benchmark --capacity 3 0.01)
endif()
