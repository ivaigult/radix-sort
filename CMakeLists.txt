cmake_minimum_required(VERSION 3.4)
project(radix-sort)

set (CMAKE_CXX_STANDARD 11)

add_executable(generate   src/generate.cpp)

add_executable(quick-sort src/sort.cpp)
target_compile_definitions(quick-sort PRIVATE SORT=std::sort)

add_executable(radix-sort src/sort.cpp)
target_compile_definitions(radix-sort PRIVATE SORT=radix_sort)

add_executable(concurrent-radix-sort src/sort.cpp)
target_compile_definitions(concurrent-radix-sort PRIVATE SORT=no_tbb::radix_sort)
 