#include "service_registry.h"
#include <cstddef>
#include <cstdint>
#include <log.h>
#include <opencv2/opencv.hpp>
#include <random>
#include <sys/types.h>

#include "vaccel.h"

grpc::Status ServiceImpl::TensorflowModelLoad(
    ::grpc::ServerContext *context,
    const ::vaccel::TensorflowModelLoadRequest *request,
    ::vaccel::TensorflowModelLoadResponse *response) {
    printf("Received TensorflowModelLoad request with model ID: %ld\n",
           request->model_id());

    return grpc::Status::OK;
}

grpc::Status ServiceImpl::TensorflowModelUnload(
    ::grpc::ServerContext *context,
    const ::vaccel::TensorflowModelUnloadRequest *request,
    ::vaccel::TensorflowModelUnloadResponse *response) {
    printf("Received TensorflowModelUnload request with model ID: %ld\n",
           request->model_id());

    return grpc::Status::OK;
}

grpc::Status ServiceImpl::TensorflowModelRun(
    ::grpc::ServerContext *context,
    const ::vaccel::TensorflowModelRunRequest *request,
    ::vaccel::TensorflowModelRunResponse *response) {
    printf("Received TensorflowModelRun request with model ID: %ld\n",
           request->model_id());

    return grpc::Status::OK;
}