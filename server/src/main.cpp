#include <cstdio>
#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <iostream>

#include "service_registry.h"

int main(int argc, char *argv[]) {
    grpc::ServerBuilder builder;
    builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());

    ServiceImpl service_impl;
    builder.RegisterService(&service_impl);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on port: 0.0.0.0:50051" << std::endl;
    server->Wait();

    return 0;
}
