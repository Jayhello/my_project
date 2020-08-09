cmake_minimum_required(VERSION 3.9)

project(demo_common)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS -pthread)

add_executable(demo_thread_pool demo_thread_pool.cpp)
target_link_libraries(demo_thread_pool common_lib)

add_executable(demo_rwlock demo_rwlock.cpp)
target_link_libraries(demo_rwlock common_lib)
