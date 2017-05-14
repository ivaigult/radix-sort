project(radix-sort)

set (CMAKE_CXX_STANDARD 11)

add_executable(generate   src/generate.cpp)
add_executable(quick-sort src/quick-sort.cpp)
add_executable(radix-sort src/radix-sort.cpp)