cmake_minimum_required(VERSION 3.9)

project(http_lib)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS -pthread)

file(GLOB HTTP_SRCS ./*.cpp ./*.h)

add_library(http_lib
        ${HTTP_SRCS})

target_link_libraries(http_lib
        common_lib
        network_lib
        )
