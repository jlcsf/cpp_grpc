#include <grpcpp/impl/channel_interface.h>
#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>
#include <memory>

#include <myproto/agent.grpc.pb.h>
#include <myproto/error.pb.h>
#include <myproto/genop.pb.h>
#include <myproto/image.pb.h>
#include <myproto/profiling.pb.h>
#include <myproto/resources.pb.h>
#include <myproto/session.pb.h>
#include <myproto/tensorflow.pb.h>
#include <myproto/torch.pb.h>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

#include "client_functions.h"

std::vector<char> ReadImageFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return {};
    }

    std::ifstream::pos_type fileSize = file.tellg();
    if (fileSize < 0) {
        std::cerr << "Error determining file size: " << filename << std::endl;
        return {};
    }

    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(fileSize);
    if (!file.read(buffer.data(), fileSize)) {
        std::cerr << "Error reading file: " << filename << std::endl;
        return {};
    }

    return buffer;
}

int main(int argc, char* argv[]) {
    auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());

    if (!channel) {
        std::cerr << "Failed to connect to the server" << std::endl;
        return 1;
    }

    int session_id = 99999;
    int resource_id = 99999;

    {
        grpc::ClientContext context;
        vaccel::CreateSessionResponse response;
        response = CreateSession(channel, context, 5); // create session with flag 5

        session_id = response.session_id();

        if (session_id != 99999) {
            std::cout << "Session ID: " << session_id << " has been created" << std::endl;
        } else {
            std::cerr << "Error: Failed to create session." << std::endl;
            return 1;
        }
    }

    {
        grpc::ClientContext context;
        vaccel::VaccelEmpty response = UpdateSession(channel, context, session_id, 1); // update session to change its flag to 1

        std::cout << "Session has been updated with the new flag" << std::endl;
    }

    {
        grpc::ClientContext context;
        vaccel::CreateResourceRequest request;
        vaccel::CreateResourceResponse response;
        request.mutable_tf();
        response = CreateResource(channel, context, request);
        resource_id = response.resource_id();
        if (resource_id == 99999) {
            std::cerr << "Error: Failed to create resource." << std::endl;
            return 1;
        }
        std::cout << "Resource created with resource ID: " << resource_id << std::endl;
    }

    {
        grpc::ClientContext context;
        vaccel::VaccelEmpty response;
        response = RegisterResource(channel, context, resource_id, session_id); // register resource to the session
        std::cout << "Resource has been registered" << std::endl;
    }

    {
        grpc::ClientContext context;
        vaccel::ImageClassificationResponse response;
        std::string image_path = "/home/jl/exmpl-cmake-grpc/client/src/example.jpg";
        std::vector<char> img_bytes = ReadImageFile(image_path);

        if (img_bytes.empty()) {
            std::cerr << "Error: Failed to read image file." << std::endl;
            return 1;
        }

        std::string img_str(img_bytes.begin(), img_bytes.end());

        response = ImageClassification(channel, context, session_id, img_str);

        std::vector<std::string> output;
        output.push_back(response.tags());

        std::cout << "-------------------------------------- " << std::endl;
        std::cout << "Output for image classification: " << std::endl;

        for (const auto& tag : output) {
            std::cout << tag << std::endl;
        }

        std::cout << "-------------------------------------- " << std::endl;
    }

    {
        grpc::ClientContext context;
        std::vector<vaccel::GenopArg> read_args;
        std::vector<vaccel::GenopArg> write_args;

        vaccel::GenopArg read_arg_op;
        read_arg_op.set_argtype(1);
        read_arg_op.set_size(sizeof(int));
        int op_value = 2; // image classify
        std::vector<char> bytes(sizeof(op_value));
        std::memcpy(bytes.data(), &op_value, sizeof(op_value));

        read_arg_op.set_buf(bytes.data(), bytes.size());
        read_args.push_back(read_arg_op);

        // Add an image
        vaccel::GenopArg read_arg_image;
        std::string image_path = "client/src/example.jpg";
        std::vector<char> img_bytes = ReadImageFile(image_path);

        if (img_bytes.empty()) {
            return 1;
        }

        std::string img_str(img_bytes.begin(), img_bytes.end());
        read_arg_image.set_argtype(1);
        read_arg_image.set_size(img_str.size());
        read_arg_image.set_buf(img_str);
        read_args.push_back(read_arg_image);

        std::string byte_data(100, ' ');
        for (int i = 0; i < 2; ++i) {
            vaccel::GenopArg write_arg;
            write_arg.set_argtype(2);
            write_arg.set_size(byte_data.size());
            write_arg.set_buf(byte_data);
            write_args.push_back(write_arg);
        }
        vaccel::GenopResponse genop_result =
            Genop(channel, context, session_id, read_args, write_args);

        std::cout << "--------------- RETURN FOR GENOP OPERATION: ---------------" << std::endl;

        for (const auto& arg : genop_result.genop_result().write_args()) {
            std::string output(reinterpret_cast<const char*>(arg.buf().data()), arg.buf().size());
            std::cout << "Output: " << output << std::endl;
        }

        std::cout << "--------------- FOR GENOP OPERATION: ---------------" << std::endl;
    }

    {
        grpc::ClientContext context;
        vaccel::VaccelEmpty response = UnregisterResource(channel, context, resource_id, session_id); // unregister the sesssion...
        std::cout << "Resource has been unregistered" << std::endl;
    }

    {
        grpc::ClientContext context;
        vaccel::VaccelEmpty result = DestroyResource(channel, context, resource_id); // destroy the resource
        std::cout << "Resource has been destroyed" << std::endl;
    }

    {
        grpc::ClientContext context;
        vaccel::VaccelEmpty result = DestroySession(channel, context, session_id); // destroy the session
        std::cout << "Session has been destroyed" << std::endl;
    }

    return 0;
}
