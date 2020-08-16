cmake_minimum_required(VERSION 3.10)
project(my_project)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS -pthread)

include_directories(./)

add_subdirectory(common)

add_subdirectory(network)

add_subdirectory(demo/demo_common)

add_subdirectory(demo/demo_network)

set(SOURCE_FILES main.cpp)
add_executable(my_project ${SOURCE_FILES})
target_link_libraries(my_project common_lib)