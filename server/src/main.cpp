#include <cstdio>
#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <iostream>

#include <myproto/agent.grpc.pb.h>
#include <myproto/error.pb.h>
#include <myproto/genop.pb.h>
#include <myproto/image.pb.h>
#include <myproto/profiling.pb.h>
#include <myproto/resources.pb.h>
#include <myproto/session.pb.h>
#include <myproto/tensorflow.pb.h>
#include <myproto/torch.pb.h>

class VaccelAgentServiceImpl final : public vaccel::VaccelAgent::Service {
public:
  // Session handling
  grpc::Status
  CreateSession(::grpc::ServerContext *context,
                const ::vaccel::CreateSessionRequest *request,
                ::vaccel::CreateSessionResponse *response) override {
    printf("Received CreateSession request with flag: %d\n", request->flags());

    uint32_t dummy_session_id = 12345;
    response->set_session_id(dummy_session_id);
    return grpc::Status::OK;
  }

  grpc::Status UpdateSession(::grpc::ServerContext *context,
                             const ::vaccel::UpdateSessionRequest *request,
                             ::vaccel::VaccelEmpty *response) override {
    printf("Received UpdateSession request with flag: %d\n", request->flags());

    return grpc::Status::OK;
  }

  grpc::Status DestroySession(::grpc::ServerContext *context,
                              const ::vaccel::DestroySessionRequest *request,
                              ::vaccel::VaccelEmpty *response) override {
    printf("Received DestroySession request with session ID: %d\n",
           request->session_id());

    return grpc::Status::OK;
  }

  grpc::Status
  CreateResource(::grpc::ServerContext *context,
                 const ::vaccel::CreateResourceRequest *request,
                 ::vaccel::CreateResourceResponse *response) override {
    printf("Received CreateResource request with type");

    uint32_t dummy_res_id = 12345;
    response->set_resource_id(dummy_res_id);

    return grpc::Status::OK;
  }

  grpc::Status
  RegisterResource(::grpc::ServerContext *context,
                   const ::vaccel::RegisterResourceRequest *request,
                   ::vaccel::VaccelEmpty *response) override {
    printf("Received RegisterResource request with resource ID: %lu and "
           "session ID: %u\n",
           request->resource_id(),
           static_cast<unsigned int>(request->session_id()));

    return grpc::Status::OK;
  }

  grpc::Status
  UnregisterResource(::grpc::ServerContext *context,
                     const ::vaccel::UnregisterResourceRequest *request,
                     ::vaccel::VaccelEmpty *response) override {
    printf("Received UnregisterResource request with resource ID: %u and "
           "session ID: %u\n",
           static_cast<unsigned int>(request->resource_id()),
           static_cast<unsigned int>(request->session_id()));
    return grpc::Status::OK;
  }

  grpc::Status DestroyResource(::grpc::ServerContext *context,
                               const ::vaccel::DestroyResourceRequest *request,
                               ::vaccel::VaccelEmpty *response) override {
    printf("Received DestroyResource request with resource ID: %ld\n",
           request->resource_id());
    return grpc::Status::OK;
  }

  grpc::Status TensorflowModelLoad(
      ::grpc::ServerContext *context,
      const ::vaccel::TensorflowModelLoadRequest *request,
      ::vaccel::TensorflowModelLoadResponse *response) override {
    printf("Received TensorflowModelLoad request with model ID: %ld\n",
           request->model_id());
    return grpc::Status::OK;
  }

  grpc::Status TensorflowModelUnload(
      ::grpc::ServerContext *context,
      const ::vaccel::TensorflowModelUnloadRequest *request,
      ::vaccel::TensorflowModelUnloadResponse *response) override {
    printf("Received TensorflowModelUnload request with model ID: %ld\n",
           request->model_id());
    return grpc::Status::OK;
  }

  grpc::Status
  TensorflowModelRun(::grpc::ServerContext *context,
                     const ::vaccel::TensorflowModelRunRequest *request,
                     ::vaccel::TensorflowModelRunResponse *response) override {
    printf("Received TensorflowModelRun request with model ID: %ld\n",
           request->model_id());
    return grpc::Status::OK;
  }

  grpc::Status ImageClassification(
      ::grpc::ServerContext *context,
      const ::vaccel::ImageClassificationRequest *request,
      ::vaccel::ImageClassificationResponse *response) override {
    printf("Received ImageClassification request\n");

    return grpc::Status::OK;
  }

  grpc::Status Genop(::grpc::ServerContext *context,
                     const ::vaccel::GenopRequest *request,
                     ::vaccel::GenopResponse *response) override {

    std::cout << "Received Genop request with session ID: "
              << request->session_id() << std::endl;

    // for (const auto& read_arg : request->read_args()) {
    //     std::cout << "Read Arg: " << read_arg.DebugString() << std::endl;
    // }

    // for (const auto& write_arg : request->write_args()) {
    //     std::cout << "Write Arg: " << write_arg.DebugString() << std::endl;
    // }

    // don't print this out for now -> floods the entire console

    return grpc::Status::OK;
  }
};

int main(int argc, char *argv[]) {
  grpc::ServerBuilder builder;
  builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());

  VaccelAgentServiceImpl service_impl;
  builder.RegisterService(&service_impl);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on port: 0.0.0.0:50051" << std::endl;
  server->Wait();

  return 0;
}
