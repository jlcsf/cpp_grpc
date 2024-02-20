#include <grpcpp/impl/channel_interface.h>
#include <memory>
#include <myproto/x_address.pb.h>
#include <myproto/x_addressbook.grpc.pb.h>

#include <myproto/agent.grpc.pb.h>
#include <myproto/error.pb.h>
#include <myproto/genop.pb.h>
#include <myproto/image.pb.h>
#include <myproto/profiling.pb.h>
#include <myproto/resources.pb.h>
#include <myproto/session.pb.h>
#include <myproto/tensorflow.pb.h>
#include <myproto/torch.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>

#include <iostream>
#include <ostream>
#include <string>
#include <fstream>


std::string CreateSession(const std::shared_ptr<grpc::Channel>& channel, grpc::ClientContext& context, int flags) {
    std::unique_ptr<vaccel::VaccelAgent::Stub> stub = vaccel::VaccelAgent::NewStub(channel);

    vaccel::CreateSessionRequest query;
    query.set_flags(flags);
    vaccel::CreateSessionResponse result;

    grpc::Status status = stub->CreateSession(&context, query, &result);

    if (status.ok()) {
        return std::to_string(result.session_id());
    } else {
        return "";
    }
}

vaccel::VaccelEmpty UpdateSession(const std::shared_ptr<grpc::Channel>& channel, grpc::ClientContext& context, int session_id, int flags) {
    std::unique_ptr<vaccel::VaccelAgent::Stub> stub = vaccel::VaccelAgent::NewStub(channel);

    vaccel::UpdateSessionRequest request;
    request.set_session_id(session_id);
    request.set_flags(flags);
    vaccel::VaccelEmpty result;

    grpc::Status status = stub->UpdateSession(&context, request, &result);

    if (status.ok()) {
        return result;  
    } else {
        std::cerr << "Error updating session: " << status.error_message() << std::endl;
        return vaccel::VaccelEmpty(); 
    }
}

vaccel::VaccelEmpty DestroySession(const std::shared_ptr<grpc::Channel>& channel, grpc::ClientContext& context, int session_id) {
    std::unique_ptr<vaccel::VaccelAgent::Stub> stub = vaccel::VaccelAgent::NewStub(channel);

    vaccel::DestroySessionRequest request;
    request.set_session_id(session_id);
    vaccel::VaccelEmpty result;

    grpc::Status status = stub->DestroySession(&context, request, &result);

    if (status.ok()) {
        return result;  
    } else {
        std::cerr << "Error destroying session: " << status.error_message() << std::endl;
        return vaccel::VaccelEmpty(); 
    }
}

std::string CreateResource(const std::shared_ptr<grpc::Channel>& channel, grpc::ClientContext& context, const vaccel::CreateResourceRequest& model) {
    std::unique_ptr<vaccel::VaccelAgent::Stub> stub = vaccel::VaccelAgent::NewStub(channel);

    vaccel::CreateResourceRequest request;
    vaccel::CreateResourceResponse response;

    if (model.has_tf()) {
        request.mutable_tf()->CopyFrom(model.tf());
    } else {
        return "Unsupported model type";
    }

    grpc::Status status = stub->CreateResource(&context, request, &response);

    if (status.ok()) {
        return std::to_string(response.resource_id());
    } else {
        // Handle error
        return "Error creating resource";
    }
}

vaccel::VaccelEmpty DestroyResource(const std::shared_ptr<grpc::Channel>& channel, grpc::ClientContext& context, int resource_id) {
    std::unique_ptr<vaccel::VaccelAgent::Stub> stub = vaccel::VaccelAgent::NewStub(channel);

    vaccel::DestroyResourceRequest request;
    request.set_resource_id(resource_id);

    vaccel::VaccelEmpty result;

    grpc::Status status = stub->DestroyResource(&context, request, &result);

    if (status.ok()) {
        std::cout << "Resource " << resource_id << " destroyed" << std::endl;
    } else {
        std::cerr << "Error: Failed to destroy resource " << resource_id << std::endl;
    }

    return result;
}

vaccel::VaccelEmpty RegisterResource(const std::shared_ptr<grpc::Channel>& channel, grpc::ClientContext& context, int resource_id, int session_id) {
    std::unique_ptr<vaccel::VaccelAgent::Stub> stub = vaccel::VaccelAgent::NewStub(channel);

    vaccel::RegisterResourceRequest request;
    request.set_resource_id(resource_id);
    request.set_session_id(session_id);

    vaccel::VaccelEmpty result;

    grpc::Status status = stub->RegisterResource(&context, request, &result);

    if (status.ok()) {
        std::cout << "Session " << session_id << " registered with resource ID " << resource_id << std::endl;
    } else {
        std::cerr << "Error: Failed to register session " << session_id << " with resource ID " << resource_id << std::endl;
    }

    return result;
}

vaccel::VaccelEmpty UnregisterResource(const std::shared_ptr<grpc::Channel>& channel, grpc::ClientContext& context, int resource_id, int session_id) {
    std::unique_ptr<vaccel::VaccelAgent::Stub> stub = vaccel::VaccelAgent::NewStub(channel);

    vaccel::UnregisterResourceRequest request;
    request.set_resource_id(resource_id);
    request.set_session_id(session_id);

    vaccel::VaccelEmpty result;

    grpc::Status status = stub->UnregisterResource(&context, request, &result);

    if (status.ok()) {
        std::cout << "Session " << session_id << " unregistered with resource ID " << resource_id << std::endl;
    } else {
        std::cerr << "Error: Failed to unregister session " << session_id << " with resource ID " << resource_id << std::endl;
    }

    return result;
}


std::vector<std::string> ImageClassification(const std::shared_ptr<grpc::Channel>& channel, grpc::ClientContext& context, int session_id, const std::string& image) {
    std::unique_ptr<vaccel::VaccelAgent::Stub> stub = vaccel::VaccelAgent::NewStub(channel);

    vaccel::ImageClassificationRequest request;
    request.set_session_id(session_id);
    request.set_image(image);

    vaccel::ImageClassificationResponse response;

    grpc::Status status = stub->ImageClassification(&context, request, &response);

    if (status.ok()) {
        std::vector<std::string> tags;
        tags.push_back(response.tags());
        return tags;
    } else {
        std::cerr << "Error: Failed to classify image" << std::endl;
        return {};
    }
}

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



int main(int argc, char* argv[])
{
    auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
    std::string session_id;
    std::string resource_id;

    {
        grpc::ClientContext context;
        session_id = CreateSession(channel, context, 5); // create session with flag 5
        
        if (!session_id.empty()) {
            std::cout << "Session ID: " << session_id <<  " has been created" << std::endl;
        } else {
            std::cout << "Error: Failed to create session." << std::endl;
        }
    }


    {
        grpc::ClientContext context;
        vaccel::VaccelEmpty result = UpdateSession(channel, context, std::stoi(session_id), 1); // update ession with flag 1
        std::cout << "Session has been updated with the new flag" << std::endl;
    }

    {

        grpc::ClientContext context;
        vaccel::CreateResourceRequest request;
        request.mutable_tf();
        resource_id = CreateResource(channel, context, request);
        std::cout << "Resource created" << std::endl;
    }

    {
        grpc::ClientContext context;
        RegisterResource(channel, context, std::stoi(resource_id), std::stoi(session_id));
        std::cout << "Resource has been registered" << std::endl;
    }


    {
        grpc::ClientContext context;
        std::string image_path = "/home/jl/exmpl-cmake-grpc/client/src/example.jpg";
        std::vector<char> img_bytes = ReadImageFile(image_path);

        if (img_bytes.empty()) {
            return 1; 
        }

        std::string img_str(img_bytes.begin(), img_bytes.end());

        std::vector<std::string> resp = ImageClassification(channel, context, std::stoi(session_id), img_str);

        for (const auto& tag : resp) {
            std::cout << tag << std::endl;
        }

    }

    {
        grpc::ClientContext context;
        UnregisterResource(channel, context, std::stoi(resource_id), std::stoi(session_id));
        std::cout << "Resource has been unregistered" << std::endl;
    }

    {
        grpc::ClientContext context;
        vaccel::VaccelEmpty result = DestroyResource(channel, context, std::stoi(resource_id));
        std::cout << "Resource has been destroyed" << std::endl;
    }

    {
        grpc::ClientContext context;
        vaccel::VaccelEmpty result = DestroySession(channel, context, std::stoi(session_id)); 
        std::cout << "Session has been destroyed" << std::endl;
    }


    return 0;
}
