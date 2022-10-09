cmake_minimum_required(VERSION 3.10)
project(my_project)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS -pthread)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")

include_directories(./)

set(BOOST_ROOT "/mnt/f/3rd/dependence/")
set(BOOST_INCLUDE_DIRS "${BOOST_ROOT}/include")
set(BOOST_LIBRARY_DIRS "${BOOST_ROOT}/lib")

find_package(Protobuf REQUIRED)
message("main Protobuf lib: ${PROTOBUF_LIBRARIES}, inc: ${PROTOBUF_INCLUDE_DIR}")

add_subdirectory(common)

add_subdirectory(network)

add_subdirectory(common_proto)

add_subdirectory(http)

# for common.pb.h
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(demo/demo_common)

add_subdirectory(demo/demo_network)

add_subdirectory(demo/demo_http)

add_subdirectory(demo/demo_raw_network)

set(SOURCE_FILES main.cpp)
add_executable(my_project ${SOURCE_FILES})
target_link_libraries(my_project common_lib)