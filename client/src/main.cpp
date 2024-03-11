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

#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

std::string CreateSession(const std::shared_ptr<grpc::Channel> &channel,
                          grpc::ClientContext &context, int flags) {
  std::unique_ptr<vaccel::VaccelAgent::Stub> stub =
      vaccel::VaccelAgent::NewStub(channel);

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

vaccel::VaccelEmpty UpdateSession(const std::shared_ptr<grpc::Channel> &channel,
                                  grpc::ClientContext &context, int session_id,
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

vaccel::VaccelEmpty
DestroySession(const std::shared_ptr<grpc::Channel> &channel,
               grpc::ClientContext &context, int session_id) {
  std::unique_ptr<vaccel::VaccelAgent::Stub> stub =
      vaccel::VaccelAgent::NewStub(channel);

  vaccel::DestroySessionRequest request;
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

std::string CreateResource(const std::shared_ptr<grpc::Channel> &channel,
                           grpc::ClientContext &context,
                           const vaccel::CreateResourceRequest &model) {
  std::unique_ptr<vaccel::VaccelAgent::Stub> stub =
      vaccel::VaccelAgent::NewStub(channel);

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

vaccel::VaccelEmpty
DestroyResource(const std::shared_ptr<grpc::Channel> &channel,
                grpc::ClientContext &context, int resource_id) {
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

vaccel::VaccelEmpty
RegisterResource(const std::shared_ptr<grpc::Channel> &channel,
                 grpc::ClientContext &context, int resource_id,
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

vaccel::VaccelEmpty
UnregisterResource(const std::shared_ptr<grpc::Channel> &channel,
                   grpc::ClientContext &context, int resource_id,
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

std::vector<std::string>
ImageClassification(const std::shared_ptr<grpc::Channel> &channel,
                    grpc::ClientContext &context, int session_id,
                    const std::string &image) {
  std::unique_ptr<vaccel::VaccelAgent::Stub> stub =
      vaccel::VaccelAgent::NewStub(channel);

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

std::vector<char> ReadImageFile(const std::string &filename) {
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

vaccel::GenopResponse Genop(const std::shared_ptr<grpc::Channel> &channel,
                            grpc::ClientContext &context, int session_id,
                            const std::vector<vaccel::GenopArg> &read_args,
                            const std::vector<vaccel::GenopArg> &write_args) {

  std::unique_ptr<vaccel::VaccelAgent::Stub> stub =
      vaccel::VaccelAgent::NewStub(channel);

  vaccel::GenopRequest request;
  request.set_session_id(session_id);

  for (const auto &read_arg : read_args) {
    auto *arg = request.add_read_args();
    arg->CopyFrom(read_arg);
  }

  for (const auto &write_arg : write_args) {
    auto *arg = request.add_write_args();
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

vaccel::TensorflowModelLoadResponse
TensorflowModelLoad(const std::shared_ptr<grpc::Channel> &channel,
                    grpc::ClientContext &context, int session_id,
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

vaccel::TensorflowModelUnloadResponse
TensorflowModelUnload(const std::shared_ptr<grpc::Channel> &channel,
                      grpc::ClientContext &context, int session_id,
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

vaccel::TensorflowModelRunResponse
TensorflowModelRun(const std::shared_ptr<grpc::Channel> &channel,
                   grpc::ClientContext &context, int session_id, int model_id,
                   const std::string &run_options,
                   const std::vector<std::pair<std::string, int>> &in_nodes,
                   const std::vector<std::pair<std::string, int>> &out_nodes,
                   const std::vector<uint8_t> &in_tensors_data) {
  std::unique_ptr<vaccel::VaccelAgent::Stub> stub =
      vaccel::VaccelAgent::NewStub(channel);

  vaccel::TensorflowModelRunRequest request;
  request.set_session_id(session_id);
  request.set_model_id(model_id);
  request.set_run_options(run_options);

  for (const auto &node_name : in_nodes) {
    vaccel::TFNode *in_node = request.add_in_nodes();
    in_node->set_name(node_name.first);
    in_node->set_id(node_name.second);
  }

  for (const auto &node_name : out_nodes) {
    vaccel::TFNode *out_node = request.add_out_nodes();
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

int main(int argc, char *argv[]) {
  auto channel = grpc::CreateChannel("localhost:50051",
                                     grpc::InsecureChannelCredentials());
  std::string session_id;
  std::string resource_id;

  {
    grpc::ClientContext context;
    session_id =
        CreateSession(channel, context, 5); // create session with flag 5

    if (!session_id.empty()) {
      std::cout << "Session ID: " << session_id << " has been created"
                << std::endl;
    } else {
      std::cout << "Error: Failed to create session." << std::endl;
    }
  }

  {
    grpc::ClientContext context;
    vaccel::VaccelEmpty result =
        UpdateSession(channel, context, std::stoi(session_id),
                      1); // update ession with flag 1
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
    RegisterResource(channel, context, std::stoi(resource_id),
                     std::stoi(session_id));
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

    std::vector<std::string> resp =
        ImageClassification(channel, context, std::stoi(session_id), img_str);

    for (const auto &tag : resp) {
      std::cout << tag << std::endl;
    }
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
    std::string image_path = "/home/jl/exmpl-cmake-grpc/client/src/example.jpg";
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
        Genop(channel, context, std::stoi(session_id), read_args, write_args);

    std::cout << "--------------- RETURN FOR GENOP OPERATION: ---------------"
              << std::endl;

    for (const auto &arg : genop_result.genop_result().write_args()) {
      std::string output(reinterpret_cast<const char *>(arg.buf().data()),
                         arg.buf().size());
      std::cout << "Output: " << output << std::endl;
    }

    std::cout << "--------------- FOR GENOP OPERATION: ---------------"
              << std::endl;
  }

  {

    grpc::ClientContext context;
    int model_id = 1;
    TensorflowModelLoad(channel, context, std::stoi(session_id), model_id);
    std::cout << "Tensorflow model has been loaded" << std::endl;
  }

  {
    grpc::ClientContext context;
    int model_id = 1;
    std::string run_options = "your_run_options_here";
    std::vector<std::pair<std::string, int>> in_nodes = {
        {"serving_default_input_1", 0}};
    std::vector<std::pair<std::string, int>> out_nodes = {
        {"StatefulPartitionedCall", 0}};
    std::vector<uint8_t> in_tensors_data = {1};
    vaccel::TensorflowModelRunResponse response =
        TensorflowModelRun(channel, context, std::stoi(session_id), model_id,
                           run_options, in_nodes, out_nodes, in_tensors_data);

    std::cout << "Tensorflow model has been ran" << std::endl;
  }

  {

    grpc::ClientContext context;
    int model_id = 1;
    TensorflowModelUnload(channel, context, std::stoi(session_id), model_id);
    std::cout << "Tensorflow model has been unloaded" << std::endl;
  }

  {
    grpc::ClientContext context;
    UnregisterResource(channel, context, std::stoi(resource_id),
                       std::stoi(session_id));
    std::cout << "Resource has been unregistered" << std::endl;
  }

  {
    grpc::ClientContext context;
    vaccel::VaccelEmpty result =
        DestroyResource(channel, context, std::stoi(resource_id));
    std::cout << "Resource has been destroyed" << std::endl;
  }

  {
    grpc::ClientContext context;
    vaccel::VaccelEmpty result =
        DestroySession(channel, context, std::stoi(session_id));
    std::cout << "Session has been destroyed" << std::endl;
  }

  return 0;
}
