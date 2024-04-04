#include <grpcpp/create_channel.h>
#include <myproto/agent.grpc.pb.h>

vaccel::CreateSessionResponse CreateSession(
    const std::shared_ptr<grpc::Channel>& channel,
    grpc::ClientContext& context,
    int flags) {
    
    std::unique_ptr<vaccel::VaccelAgent::Stub> stub =
        vaccel::VaccelAgent::NewStub(channel);

    vaccel::CreateSessionRequest query;
    query.set_flags(flags);
    vaccel::CreateSessionResponse response;

    grpc::Status status = stub->CreateSession(&context, query, &response);

    if (status.ok()) {
        return response;
    } else {
        std::cerr << "Error: Failed to create a session" << std::endl;
        return response;
    }
}

vaccel::VaccelEmpty UpdateSession(
    const std::shared_ptr<grpc::Channel>& channel,
    grpc::ClientContext& context,
    int session_id,
    int flags) {
    
    std::unique_ptr<vaccel::VaccelAgent::Stub> stub =
        vaccel::VaccelAgent::NewStub(channel);
    vaccel::UpdateSessionRequest request;
    request.set_session_id(session_id);
    request.set_flags(flags);
    vaccel::VaccelEmpty result;

    grpc::Status status = stub->UpdateSession(&context, request, &result);

    if (status.ok()) {
        return result;
    } else {
        std::cerr << "Error updating session: " << status.error_message()
                  << std::endl;
        return vaccel::VaccelEmpty();
    }
}

vaccel::VaccelEmpty DestroySession(
    const std::shared_ptr<grpc::Channel>& channel,
    grpc::ClientContext& context,
    int session_id) {
    
    vaccel::DestroySessionRequest request;
    std::unique_ptr<vaccel::VaccelAgent::Stub> stub =
        vaccel::VaccelAgent::NewStub(channel);

    request.set_session_id(session_id);
    vaccel::VaccelEmpty result;

    grpc::Status status = stub->DestroySession(&context, request, &result);

    if (status.ok()) {
        return result;
    } else {
        std::cerr << "Error destroying session: " << status.error_message()
                  << std::endl;
        return vaccel::VaccelEmpty();
    }
}

vaccel::CreateResourceResponse CreateResource(
    const std::shared_ptr<grpc::Channel>& channel,
    grpc::ClientContext& context,
    const vaccel::CreateResourceRequest& model) {
    
    std::unique_ptr<vaccel::VaccelAgent::Stub> stub =
        vaccel::VaccelAgent::NewStub(channel);

    vaccel::CreateResourceRequest request;
    vaccel::CreateResourceResponse response;

    if (model.has_tf()) {
        request.mutable_tf()->CopyFrom(model.tf());
    } else {
        std::cerr << "Error creating a session" << std::endl;
    }

    grpc::Status status = stub->CreateResource(&context, request, &response);

    if (status.ok()) {
        return response;
    } else {
        std::cerr << "Error: Failed to create a resource" << std::endl;
        return response;
    }
}

vaccel::VaccelEmpty DestroyResource(
    const std::shared_ptr<grpc::Channel>& channel,
    grpc::ClientContext& context,
    int resource_id) {
    
    std::unique_ptr<vaccel::VaccelAgent::Stub> stub =
        vaccel::VaccelAgent::NewStub(channel);

    vaccel::DestroyResourceRequest request;
    request.set_resource_id(resource_id);

    vaccel::VaccelEmpty result;

    grpc::Status status = stub->DestroyResource(&context, request, &result);

    if (status.ok()) {
        std::cout << "Resource " << resource_id << " destroyed" << std::endl;
    } else {
        std::cerr << "Error: Failed to destroy resource " << resource_id
                  << std::endl;
    }

    return result;
}

vaccel::VaccelEmpty RegisterResource(
    const std::shared_ptr<grpc::Channel>& channel,
    grpc::ClientContext& context,
    int resource_id,
    int session_id) {
    
    std::unique_ptr<vaccel::VaccelAgent::Stub> stub =
        vaccel::VaccelAgent::NewStub(channel);
    vaccel::RegisterResourceRequest request;
    request.set_resource_id(resource_id);
    request.set_session_id(session_id);

    vaccel::VaccelEmpty result;

    grpc::Status status = stub->RegisterResource(&context, request, &result);

    if (status.ok()) {
        std::cout << "Session " << session_id << " registered with resource ID "
                  << resource_id << std::endl;
    } else {
        std::cerr << "Error: Failed to register session " << session_id
                  << " with resource ID " << resource_id << std::endl;
    }

    return result;
}

vaccel::VaccelEmpty UnregisterResource(
    const std::shared_ptr<grpc::Channel>& channel,
    grpc::ClientContext& context,
    int resource_id,
    int session_id) {
    
    std::unique_ptr<vaccel::VaccelAgent::Stub> stub =
        vaccel::VaccelAgent::NewStub(channel);

    vaccel::UnregisterResourceRequest request;
    request.set_resource_id(resource_id);
    request.set_session_id(session_id);

    vaccel::VaccelEmpty result;

    grpc::Status status = stub->UnregisterResource(&context, request, &result);

    if (status.ok()) {
        std::cout << "Session " << session_id << " unregistered with resource ID "
                  << resource_id << std::endl;
    } else {
        std::cerr << "Error: Failed to unregister session " << session_id
                  << " with resource ID " << resource_id << std::endl;
    }

    return result;
}

vaccel::ImageClassificationResponse ImageClassification(
    const std::shared_ptr<grpc::Channel>& channel,
    grpc::ClientContext& context,
    int session_id,
    const std::string& image) {
    
    std::unique_ptr<vaccel::VaccelAgent::Stub> stub =
        vaccel::VaccelAgent::NewStub(channel);

    vaccel::ImageClassificationRequest request;
    request.set_session_id(session_id);
    request.set_image(image);

    vaccel::ImageClassificationResponse response;

    grpc::Status status = stub->ImageClassification(&context, request, &response);

    if (status.ok()) {
        return response;
    } else {
        std::cerr << "Error: Failed to create an image classification request" << std::endl;
        return response;
    }
}

vaccel::GenopResponse Genop(
    const std::shared_ptr<grpc::Channel>& channel,
    grpc::ClientContext& context,
    int session_id,
    const std::vector<vaccel::GenopArg>& read_args,
    const std::vector<vaccel::GenopArg>& write_args) {
    
    std::unique_ptr<vaccel::VaccelAgent::Stub> stub =
        vaccel::VaccelAgent::NewStub(channel);

    vaccel::GenopRequest request;
    request.set_session_id(session_id);

    for (const auto& read_arg : read_args) {
        auto* arg = request.add_read_args();
        arg->CopyFrom(read_arg);
    }

    for (const auto& write_arg : write_args) {
        auto* arg = request.add_write_args();
        arg->CopyFrom(write_arg);
    }

    vaccel::GenopResponse response;

    grpc::Status status = stub->Genop(&context, request, &response);

    if (status.ok()) {
        std::cout << "Genop request successful" << std::endl;
    } else {
        std::cerr << "Error: Genop request failed: " << status.error_message()
                  << std::endl;
    }

    return response;
}

vaccel::TensorflowModelLoadResponse TensorflowModelLoad(
    const std::shared_ptr<grpc::Channel>& channel,
    grpc::ClientContext& context,
    int session_id,
    int model_id) {
    
    std::unique_ptr<vaccel::VaccelAgent::Stub> stub =
        vaccel::VaccelAgent::NewStub(channel);

    vaccel::TensorflowModelLoadRequest request;
    request.set_session_id(session_id);
    request.set_model_id(model_id);

    vaccel::TensorflowModelLoadResponse response;

    grpc::Status status = stub->TensorflowModelLoad(&context, request, &response);

    if (status.ok()) {
        return response;
    } else {
        std::cerr << "Error: Failed to load TensorFlow model" << std::endl;
        return response;
    }
}

vaccel::TensorflowModelUnloadResponse TensorflowModelUnload(
    const std::shared_ptr<grpc::Channel>& channel,
    grpc::ClientContext& context,
    int session_id,
    int model_id) {
    
    std::unique_ptr<vaccel::VaccelAgent::Stub> stub =
        vaccel::VaccelAgent::NewStub(channel);

    vaccel::TensorflowModelUnloadRequest request;
    request.set_session_id(session_id);
    request.set_model_id(model_id);

    vaccel::TensorflowModelUnloadResponse response;

    grpc::Status status =
        stub->TensorflowModelUnload(&context, request, &response);

    if (status.ok()) {
        return response;
    } else {
        std::cerr << "Error: Failed to unload TensorFlow model" << std::endl;
        return response;
    }
}

vaccel::TensorflowModelRunResponse TensorflowModelRun(
    const std::shared_ptr<grpc::Channel>& channel,
    grpc::ClientContext& context,
    int session_id,
    int model_id,
    const std::string& run_options,
    const std::vector<std::pair<std::string, int>>& in_nodes,
    const std::vector<std::pair<std::string, int>>& out_nodes,
    const std::vector<uint8_t>& in_tensors_data) {
    
    std::unique_ptr<vaccel::VaccelAgent::Stub> stub =
        vaccel::VaccelAgent::NewStub(channel);

    vaccel::TensorflowModelRunRequest request;
    request.set_session_id(session_id);
    request.set_model_id(model_id);
    request.set_run_options(run_options);

    for (const auto& node_name : in_nodes) {
        vaccel::TFNode* in_node = request.add_in_nodes();
        in_node->set_name(node_name.first);
        in_node->set_id(node_name.second);
    }

    for (const auto& node_name : out_nodes) {
        vaccel::TFNode* out_node = request.add_out_nodes();
        out_node->set_name(node_name.first);
        out_node->set_id(node_name.second);
    }

    vaccel::TFTensor tf_tensor;
    std::string data(30, 1);
    tf_tensor.set_data(data);
    tf_tensor.add_dims(1);
    tf_tensor.add_dims(30);
    tf_tensor.set_type(vaccel::TFDataType::FLOAT);
    *request.add_in_tensors() = tf_tensor;

    vaccel::TensorflowModelRunResponse response;

    grpc::Status status = stub->TensorflowModelRun(&context, request, &response);

    if (status.ok()) {
        return response;
    } else {
        std::cerr << "Error: Failed to run TensorFlow model" << std::endl;
        return response;
    }
}
