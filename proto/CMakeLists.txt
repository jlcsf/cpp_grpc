set(PROTOC_EXECUTABLE "/usr/bin/protoc-25.1.0")
find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Threads)


set(PROTO_FILES
    myproto/error.proto
    myproto/genop.proto
    myproto/image.proto
    myproto/profiling.proto
    myproto/resources.proto
    myproto/session.proto
    myproto/tensorflow.proto
    myproto/torch.proto
    myproto/agent.proto
)


add_library(myproto ${PROTO_FILES})
target_link_libraries(myproto
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)
target_include_directories(myproto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})


get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET myproto LANGUAGE cpp)
protobuf_generate(TARGET myproto LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")
