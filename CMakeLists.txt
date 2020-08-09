cmake_minimum_required(VERSION 3.10)
project(my_project)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(common)

set(SOURCE_FILES main.cpp)
add_executable(my_project ${SOURCE_FILES})
target_link_libraries(my_project common_lib)