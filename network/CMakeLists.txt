cmake_minimum_required(VERSION 3.9)

project(network_lib)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS -pthread)

file(GLOB NETWORK_SRCS ./*.cpp ./*.h)

add_library(network_lib
        ${NETWORK_SRCS})

target_link_libraries(network_lib
        common_lib
        )