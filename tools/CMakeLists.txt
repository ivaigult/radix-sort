add_executable(generate generate.cpp)

add_executable(quick-sort sort.cpp)
target_compile_definitions(quick-sort PRIVATE SORT=std::sort)
target_link_libraries(quick-sort radix_sort)

add_executable(radix-sort sort.cpp)
target_compile_definitions(radix-sort PRIVATE SORT=radix_sort::sort)
target_link_libraries(radix-sort radix_sort)

add_executable(concurrent-radix-sort sort.cpp)
target_compile_definitions(concurrent-radix-sort PRIVATE SORT=radix_sort::concurrent_sort)
target_link_libraries(concurrent-radix-sort radix_sort)
