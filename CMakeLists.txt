cmake_minimum_required(VERSION 3.4)
project(radix-sort)

set (CMAKE_CXX_STANDARD 11)

add_executable(generate   src/generate.cpp)

add_executable(quick-sort src/sort.cpp)
target_compile_definitions(quick-sort PRIVATE SORT=std::sort)

add_executable(radix-sort src/sort.cpp)
target_compile_definitions(radix-sort PRIVATE SORT=radix_sort)

add_executable(no-tbb-radix-sort src/sort.cpp)
target_compile_definitions(no-tbb-radix-sort PRIVATE SORT=no_tbb::radix_sort)

add_executable(no-list-radix-sort src/sort.cpp)
target_compile_definitions(no-list-radix-sort PRIVATE SORT=no_list::radix_sort)

add_executable(no-tbb-and-list-sort src/sort.cpp)
target_compile_definitions(no-tbb-and-list-sort PRIVATE SORT=no_list_and_tbb::radix_sort)