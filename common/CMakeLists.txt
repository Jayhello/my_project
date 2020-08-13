cmake_minimum_required(VERSION 3.9)

project(common_lib)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS -pthread)

file(GLOB COMMON_SRCS ./*.cpp ./*.h)

add_library(common_lib
        ${COMMON_SRCS})

#find_package(PkgConfig)
#pkg_check_modules(YAMLCPP REQUIRED yaml-cpp>=0.5)
#include_directories(${YAML_CPP_INCLUDE_DIR})
#message("yaml cpp: ${YAML_CPP_INCLUDE_DIR}, lib: ${YAML_CPP_LIBRARIES} ${YAMLCPP_LIBRARIES}")
