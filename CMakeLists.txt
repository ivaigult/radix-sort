cmake_minimum_required(VERSION 3.4)
project(radix-sort)

set (CMAKE_CXX_STANDARD 11)

add_library(radix_sort INTERFACE)
target_include_directories(radix_sort INTERFACE include)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(TBB_ROOT "C:/Users/sys_avctests/Downloads/tbb2017_20170412oss_win/tbb2017_20170412oss")
find_package(Threads)
find_package(TBB)

option(BUILD_TOOLS "Enable building tools e.g. test/benchmarks/CLI" ON)
if (BUILD_TOOLS)
    add_subdirectory(tools)
endif()

