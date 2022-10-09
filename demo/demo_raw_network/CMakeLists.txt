cmake_minimum_required(VERSION 3.9)

project(demo_raw_network)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS -pthread)

#find_package(Boost REQUIRED regex date_time thread system filesystem program_options)
#include_directories(${BOOST_INCLUDE_DIRS})
#message("boost lib: ${Boost_LIBRARIES}")
#message("boost inc:${Boost_INCLUDE_DIR}")
#message("network_lib :${network_lib}")
#
#find_package(Protobuf REQUIRED)
#message("main Protobuf lib: ${PROTOBUF_LIBRARIES}, inc: ${PROTOBUF_INCLUDE_DIR}")

function(AddTestTarget test_exe test_src)
    add_executable(${test_exe} ${test_src} raw_comm.cpp)
    target_link_libraries(
            ${test_exe}
            common_lib
    )
endfunction()

AddTestTarget(raw_client_1 raw_client_1.cpp)
AddTestTarget(raw_server_1 raw_server_1.cpp)
