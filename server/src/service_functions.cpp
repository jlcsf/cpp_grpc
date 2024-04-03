#include "service_registry.h"

grpc::Status ServiceImpl::Genop(::grpc::ServerContext *context,
                                const ::vaccel::GenopRequest *request,
                                ::vaccel::GenopResponse *response) {
    printf("Received Genop request with session ID: %d\n", request->session_id());

    return grpc::Status::OK;
}

grpc::Status ServiceImpl::ImageClassification(::grpc::ServerContext *context,
                                               const ::vaccel::ImageClassificationRequest *request,
                                               ::vaccel::ImageClassificationResponse *response) {
    printf("Received ImageClassification request\n");

    return grpc::Status::OK;
}

grpc::Status ServiceImpl::CreateResource(::grpc::ServerContext *context,
                                         const ::vaccel::CreateResourceRequest *request,
                                         ::vaccel::CreateResourceResponse *response) {
    printf("Received CreateResource request with type\n");

    uint32_t dummy_res_id = 12345;
    response->set_resource_id(dummy_res_id);

    return grpc::Status::OK;
}

grpc::Status ServiceImpl::RegisterResource(::grpc::ServerContext *context,
                                           const ::vaccel::RegisterResourceRequest *request,
                                           ::vaccel::VaccelEmpty *response) {
    printf("Received RegisterResource request with resource ID: %lu and session ID: %u\n",
           request->resource_id(), static_cast<unsigned int>(request->session_id()));

    return grpc::Status::OK;
}

grpc::Status ServiceImpl::UnregisterResource(::grpc::ServerContext *context,
                                             const ::vaccel::UnregisterResourceRequest *request,
                                             ::vaccel::VaccelEmpty *response) {
    printf("Received UnregisterResource request with resource ID: %u and session ID: %u\n",
           static_cast<unsigned int>(request->resource_id()), static_cast<unsigned int>(request->session_id()));

    return grpc::Status::OK;
}

grpc::Status ServiceImpl::DestroyResource(::grpc::ServerContext *context,
                                          const ::vaccel::DestroyResourceRequest *request,
                                          ::vaccel::VaccelEmpty *response) {
    printf("Received DestroyResource request with resource ID: %ld\n", request->resource_id());

    return grpc::Status::OK;
}

grpc::Status ServiceImpl::CreateSession(::grpc::ServerContext *context,
                                         const ::vaccel::CreateSessionRequest *request,
                                         ::vaccel::CreateSessionResponse *response) {
    printf("Received CreateSession request with flag: %d\n", request->flags());

    uint32_t dummy_session_id = 12345;
    response->set_session_id(dummy_session_id);

    return grpc::Status::OK;
}

grpc::Status ServiceImpl::UpdateSession(::grpc::ServerContext *context,
                                        const ::vaccel::UpdateSessionRequest *request,
                                        ::vaccel::VaccelEmpty *response) {
    printf("Received UpdateSession request with flag: %d\n", request->flags());

    return grpc::Status::OK;
}

grpc::Status ServiceImpl::DestroySession(::grpc::ServerContext *context,
                                          const ::vaccel::DestroySessionRequest *request,
                                          ::vaccel::VaccelEmpty *response) {
    printf("Received DestroySession request with session ID: %d\n",
           request->session_id());

    return grpc::Status::OK;
}

grpc::Status ServiceImpl::TensorflowModelLoad(::grpc::ServerContext *context,
                                               const ::vaccel::TensorflowModelLoadRequest *request,
                                               ::vaccel::TensorflowModelLoadResponse *response) {
    printf("Received TensorflowModelLoad request with model ID: %ld\n", request->model_id());

    return grpc::Status::OK;
}

grpc::Status ServiceImpl::TensorflowModelUnload(::grpc::ServerContext *context,
                                                 const ::vaccel::TensorflowModelUnloadRequest *request,
                                                 ::vaccel::TensorflowModelUnloadResponse *response) {
    printf("Received TensorflowModelUnload request with model ID: %ld\n", request->model_id());

    return grpc::Status::OK;
}

grpc::Status ServiceImpl::TensorflowModelRun(::grpc::ServerContext *context,
                                              const ::vaccel::TensorflowModelRunRequest *request,
                                              ::vaccel::TensorflowModelRunResponse *response) {
    printf("Received TensorflowModelRun request with model ID: %ld\n", request->model_id());

    return grpc::Status::OK;
}
