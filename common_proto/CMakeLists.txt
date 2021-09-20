project(proto_lib)
cmake_minimum_required (VERSION 3.9)

protobuf_generate_cpp(PROTO_SRC PROTO_HEADER common.proto)
add_library(proto_lib
        ${PROTO_HEADER}
        ${PROTO_SRC}
        )


