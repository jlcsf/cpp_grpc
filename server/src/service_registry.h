#ifndef SERVICE_REGISTRY_H
#define SERVICE_REGISTRY_H

#include <cstdint>
#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <myproto/agent.grpc.pb.h>
#include <myproto/error.pb.h>
#include <myproto/genop.pb.h>
#include <myproto/image.pb.h>
#include <myproto/profiling.pb.h>
#include <myproto/resources.pb.h>
#include <myproto/session.pb.h>
#include <myproto/tensorflow.pb.h>
#include <myproto/torch.pb.h>


#include <stdexcept>
#include <sys/types.h>
#include <unordered_map>
#include "vaccel.h"

class ServiceImpl final : public vaccel::VaccelAgent::Service {
public:

    using SessionMap = std::unordered_map<uint32_t, vaccel_session>;
    using ResourceMap = std::unordered_map<uint32_t, vaccel_resource*>;

    ServiceImpl() {}

    // Genop operation
    grpc::Status Genop(::grpc::ServerContext *context,
                       const ::vaccel::GenopRequest *request,
                       ::vaccel::GenopResponse *response) override;

    // Image classification
    grpc::Status ImageClassification(::grpc::ServerContext *context,
                        const ::vaccel::ImageClassificationRequest *request,
                        ::vaccel::ImageClassificationResponse *response) override;

    // Register resource
    grpc::Status RegisterResource(::grpc::ServerContext *context,
                                   const ::vaccel::RegisterResourceRequest *request,
                                   ::vaccel::VaccelEmpty *response) override;

    // Unregister resource
    grpc::Status UnregisterResource(::grpc::ServerContext *context,
                                     const ::vaccel::UnregisterResourceRequest *request,
                                     ::vaccel::VaccelEmpty *response) override;

    // Destroy resource
    grpc::Status DestroyResource(::grpc::ServerContext *context,
                                  const ::vaccel::DestroyResourceRequest *request,
                                  ::vaccel::VaccelEmpty *response) override;

    // Create resource 
    grpc::Status CreateResource(::grpc::ServerContext *context,
                                 const ::vaccel::CreateResourceRequest *request,
                                 ::vaccel::CreateResourceResponse *response) override;
    
    // Create session
    grpc::Status CreateSession(::grpc::ServerContext *context,
                                const ::vaccel::CreateSessionRequest *request,
                                ::vaccel::CreateSessionResponse *response) override;
    
    // Update session
    grpc::Status UpdateSession(::grpc::ServerContext *context,
                                const ::vaccel::UpdateSessionRequest *request,
                                ::vaccel::VaccelEmpty *response) override;
    
    // Destroy session
    grpc::Status DestroySession(::grpc::ServerContext *context,
                                const ::vaccel::DestroySessionRequest *request,
                                ::vaccel::VaccelEmpty *response) override;

    // Load TensorFlow model
    grpc::Status TensorflowModelLoad(::grpc::ServerContext *context,
                                      const ::vaccel::TensorflowModelLoadRequest *request,
                                      ::vaccel::TensorflowModelLoadResponse *response) override;

    // Unload TensorFlow model
    grpc::Status TensorflowModelUnload(::grpc::ServerContext *context,
                                        const ::vaccel::TensorflowModelUnloadRequest *request,
                                        ::vaccel::TensorflowModelUnloadResponse *response) override;

    // Run TensorFlow model
    grpc::Status TensorflowModelRun(::grpc::ServerContext *context,
                                     const ::vaccel::TensorflowModelRunRequest *request,
                                     ::vaccel::TensorflowModelRunResponse *response) override;


    // Torch JIT Forward Load
    grpc::Status TorchJitloadForward(::grpc::ServerContext *context,
                                     const ::vaccel::TorchJitloadForwardRequest *request,
                                     ::vaccel::TorchJitloadForwardResponse *response) override ;

    // Torch JIT Forward Load
    grpc::Status TorchLoadModel(::grpc::ServerContext *context,
                                     const ::vaccel::TorchJitLoadModelFromPathRequest *request,
                                     ::vaccel::TorchJitLoadModelFromPathResponse *response) override ;

    grpc::Status TorchRegisterModel(::grpc::ServerContext *context,
                                     const ::vaccel::TorchJitRegisterModelRequest *request,
                                     ::vaccel::VaccelEmpty *response) override ;

    grpc::Status ImagePose(::grpc::ServerContext *context,
                                     const ::vaccel::ImagePoseRequest *request,
                                     ::vaccel::ImagePoseResponse *response) override ;

private:

    SessionMap sessions_map;
    ResourceMap resources_map;

};


#endif // SERVICE_REGISTRY_H
