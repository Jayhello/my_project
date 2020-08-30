project(proto_lib)
cmake_minimum_required (VERSION 3.9)

find_package(Protobuf REQUIRED)

message("main Protobuf lib: ${PROTOBUF_LIBRARIES}, inc: ${PROTOBUF_INCLUDE_DIR}")

protobuf_generate_cpp(PROTO_SRC PROTO_HEADER common.proto)
add_library(proto_lib
        ${PROTO_HEADER}
        ${PROTO_SRC}
        )


