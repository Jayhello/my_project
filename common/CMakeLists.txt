cmake_minimum_required(VERSION 3.9)

project(common_lib)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS -pthread)

file(GLOB COMMON_SRCS ./*.cpp ./*.h)

add_library(common_lib
        ${COMMON_SRCS})
