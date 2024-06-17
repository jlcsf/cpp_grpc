#include "service_registry.h"
#include <cstddef>
#include <cstdint>
#include <log.h>
#include <opencv2/opencv.hpp>
#include <random>
#include <resources/single_model.h>
#include <sys/types.h>
#include "vaccel.h"

const char *MODEL_PATH = "/home/jl/vaccel-torch-cv-example/";

void ConvertToVaccelTorchTensor(
    const std::vector<vaccel::TorchTensor> &tensor_vector,
    struct vaccel_torch_tensor **in, size_t nr_inputs) {

    for (size_t i = 0; i < nr_inputs; ++i) {
        const vaccel::TorchTensor &tensor = tensor_vector[i];

        in[i]->nr_dims = tensor.dims_size();

        in[i]->dims = new int32_t[in[i]->nr_dims];
        for (int j = 0; j < in[i]->nr_dims; ++j) {
            in[i]->dims[j] = tensor.dims(j);
        }

        in[i]->data = new char[tensor.data().size()];
        memcpy(in[i]->data, tensor.data().data(), tensor.data().size());
        in[i]->size = tensor.data().size();

        in[i]->data_type = static_cast<vaccel_torch_data_type>(tensor.type());
    }
}

struct OutputData {
    float *data; // Pointer to the output data array
    size_t size; // Size of the data array
};

grpc::Status ServiceImpl::TorchJitloadForward(
    ::grpc::ServerContext *context,
    const ::vaccel::TorchJitloadForwardRequest *request,
    ::vaccel::TorchJitloadForwardResponse *response) {

    vaccel_debug("Received TorchJitloadForward request with model ID: %ld\n",
           request->model_id());
    int session_id = request->session_id();
    int model_id = request->model_id();
    std::string bytes = request->run_options();

    int tensor_count = request->in_tensors_size();

    struct vaccel_torch_tensor **in =
        new struct vaccel_torch_tensor *[tensor_count];
    for (size_t i = 0; i < tensor_count; ++i) {
        in[i] = new struct vaccel_torch_tensor;
    }
    std::vector<vaccel::TorchTensor> tensor_vector;
    for (int i = 0; i < tensor_count; ++i) {
        tensor_vector.push_back(request->in_tensors(i));
    }
    ConvertToVaccelTorchTensor(tensor_vector, in, tensor_count);

    struct vaccel_torch_tensor *out;
    out = (struct vaccel_torch_tensor *)malloc(
        sizeof(struct vaccel_torch_tensor) * 1);

    auto it = sessions_map.find(session_id);
    if (it == sessions_map.end()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND,
                            "Session ID not found");
    }

    struct vaccel_session *sess_ptr = &(it->second);
    struct vaccel_torch_buffer run_options = {0};
    int ret;

    std::string run_options_str = request->run_options();
    run_options.data = (char *)malloc(run_options_str.size() + 1);
    run_options.size = run_options_str.size() + 1;
    strcpy(run_options.data, run_options_str.c_str());

    const char *model_path = MODEL_PATH;

    auto model_it = model_map.find(model_path);
    if (model_it == model_map.end()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND,
                            "Model pointer not found");
    }

    // Deep copy the model to avoid issues with lifetime
    struct vaccel_single_model model = *(model_it->second);

    vaccel_debug("Registered model ID with session pointer");
    vaccel_debug("Created input tensors");
    vaccel_debug("Start");

    ret = vaccel_torch_jitload_forward(sess_ptr, &model, &run_options, in,
                                       tensor_count, &out, 1);
    if (ret != 0) {
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Failed to run vaccel_torch_jitload_forward");
    }

    vaccel_debug("Processing output tensors directly from the `out` array");

    // Encode the output tensor into the gRPC response
    vaccel::TorchJitloadForwardResult *result =
        response->mutable_torch_result();
    for (int i = 0; i < 1; ++i) {
        vaccel::TorchTensor *out_tensor = result->add_out_tensors();
        for (int j = 0; j < out[i].nr_dims; ++j) {
            out_tensor->add_dims(out[i].dims[j]);
        }
        out_tensor->set_data(out[i].data, out[i].size);
        out_tensor->set_type(
            static_cast<vaccel::TorchDataType>(out[i].data_type));
    }

    vaccel_debug("Completed");

    return grpc::Status::OK;
}

grpc::Status ServiceImpl::TorchLoadModel(
    ::grpc::ServerContext *context,
    const ::vaccel::TorchJitLoadModelFromPathRequest *request,
    ::vaccel::TorchJitLoadModelFromPathResponse *response) {

    const std::string &model_path_bytes = request->model_path();
    int ret;

    const char *model_path = MODEL_PATH;
    struct vaccel_single_model model;
    
    vaccel_debug("Received TorchLoadModel request\n");

    ret = vaccel_single_model_set_path(&model, model_path);

    if (ret) {
        vaccel_debug("Could not set model path to Torch model");
        exit(1);
    }

    model_map[model_path] = new vaccel_single_model;

    vaccel_debug("Registered model %lld\n",
                 vaccel_single_model_get_id(&model));

    return grpc::Status::OK;
}

grpc::Status ServiceImpl::TorchRegisterModel(
    ::grpc::ServerContext *context,
    const ::vaccel::TorchJitRegisterModelRequest *request,
    ::vaccel::VaccelEmpty *response) {

    int model_id = request->model_id();
    int ret;
    int session_id = request->session_id();
    const char *model_path = MODEL_PATH;

    auto model_it = model_map.find(model_path);
    if (model_it == model_map.end()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND,
                            "Model pointer not found");
    }

    auto it = sessions_map.find(session_id);
    if (it == sessions_map.end()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND,
                            "Session ID not found");
    }

    struct vaccel_session *sess_ptr = &(it->second);
    struct vaccel_single_model model = *(model_it->second);

    vaccel_debug("Received TorchRegisterModel request\n");

    ret = vaccel_single_model_register(&model);
    if (ret != 0) {
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Failed to register model");
    }

    vaccel_debug("Registered model %lld\n",
                 vaccel_single_model_get_id(&model));


    ret = vaccel_sess_register(sess_ptr, model.resource);
    if (ret) {
        fprintf(stderr, "Could not register model with session\n");
        exit(1);

    }
    vaccel_debug("Registered model with session");

    return grpc::Status::OK;
}