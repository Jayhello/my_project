cmake_minimum_required(VERSION 3.9)

project(demo_network)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS -pthread)

find_package(Boost REQUIRED regex date_time thread system filesystem program_options)
include_directories(${BOOST_INCLUDE_DIRS})
message("boost lib: ${Boost_LIBRARIES}")
message("boost inc:${Boost_INCLUDE_DIR}")
message("network_lib :${network_lib}")

find_package(Protobuf REQUIRED)
message("main Protobuf lib: ${PROTOBUF_LIBRARIES}, inc: ${PROTOBUF_INCLUDE_DIR}")

add_executable(demo_network demo_network.cpp)
target_link_libraries(demo_network
        common_lib
        network_lib
        proto_lib
        ${PROTOBUF_LIBRARIES}
        )

add_executable(demo_client demo_client.cpp)
target_link_libraries(demo_client
        common_lib
        network_lib
        proto_lib
        ${PROTOBUF_LIBRARIES}
        )

add_executable(demo_service demo_service.cpp)
target_link_libraries(demo_service
        common_lib
        network_lib
        proto_lib
        ${PROTOBUF_LIBRARIES}
        )
