cmake_minimum_required(VERSION 3.9)

project(demo_network)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS -pthread)

find_package(Boost REQUIRED regex date_time thread system filesystem program_options)
include_directories(${BOOST_INCLUDE_DIRS})
message("boost lib: ${Boost_LIBRARIES}")
message("boost inc:${Boost_INCLUDE_DIR}")
message("network_lib :${network_lib}")


add_executable(demo_http demo_http.cpp)
target_link_libraries(demo_http
        common_lib
        network_lib
        proto_lib
        http_lib
        ${PROTOBUF_LIBRARIES}
        )
