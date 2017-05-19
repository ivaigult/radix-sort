cmake_minimum_required(VERSION 3.4)
project(radix-sort)

set (CMAKE_CXX_STANDARD 11)

add_library(radix_sort INTERFACE)
target_include_directories(radix_sort INTERFACE include)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

add_library(itt_notify INTERFACE)
target_include_directories(itt_notify INTERFACE "C:/Program Files (x86)/IntelSWTools/VTune Amplifier 2018/include")
target_link_libraries(itt_notify INTERFACE "C:/Program Files (x86)/IntelSWTools/VTune Amplifier 2018/lib64/libittnotify.lib")

option(BUILD_TOOLS "Enable building tools e.g. test/benchmarks/CLI" ON)
if (BUILD_TOOLS)
    add_subdirectory(tools)
endif()

