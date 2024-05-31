#include "service_registry.h"
#include "vaccel.h"
#include <cstdint>
#include <log.h>
#include <opencv2/opencv.hpp>
#include <ops/torch.h>
#include <ops/vaccel_ops.h>
#include <random>
#include <session.h>

grpc::Status ServiceImpl::Genop(::grpc::ServerContext *context,
                                const ::vaccel::GenopRequest *request,
                                ::vaccel::GenopResponse *response) {
    printf("Received Genop request with session ID: %d\n",
           request->session_id());

    int len_read = 0;
    int len_write = 0;

    // Retrieve read arguments
    printf("Read arguments:\n");
    for (const auto &arg : request->read_args()) {
        printf("  Argument type: %d, Size: %d\n", arg.argtype(), arg.size());
        len_read += 1;
    }

    // Retrieve write arguments
    printf("Write arguments:\n");
    for (const auto &arg : request->write_args()) {
        printf("  Argument type: %d, Size: %d\n", arg.argtype(), arg.size());
        len_write += 1;
    }

    printf("Total length of read arguments: %d\n", len_read);
    printf("Total length of write arguments: %d\n", len_write);

    uint32_t session_id = request->session_id();

    auto it = sessions_map.find(session_id);
    if (it == sessions_map.end()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND,
                            "Session ID not found");
    }

    vaccel_session *sess_ptr = &(it->second);

    struct vaccel_arg read[len_read];
    struct vaccel_arg write[len_write];
    enum vaccel_op_type op_type;

    const vaccel::GenopArg &genop_arg = request->read_args(0);
    const std::string &byte_sequence = genop_arg.buf();

    if (byte_sequence.size() >= sizeof(std::uint32_t)) {
        std::uint32_t value;
        std::memcpy(&value, byte_sequence.data(), sizeof(value));

        op_type = static_cast<vaccel_op_type>(value);
    } else {
        // byte sequence does not contain enough bytes
    }

    read[0].size = sizeof(enum vaccel_op_type);
    read[0].buf = &op_type;

    for (size_t i = 1; i < len_read; ++i) {
        const std::string &arg_buf = request->read_args(i).buf();
        read[i].size = arg_buf.size();
        read[i].buf = const_cast<char *>(arg_buf.data());
    }

    for (size_t i = 0; i < len_write; ++i) {
        const std::string &arg_buf = request->write_args(i).buf();
        write[i].size = arg_buf.size();
        write[i].buf = const_cast<char *>(arg_buf.data());
    }

    int ret;
    ret = vaccel_genop(sess_ptr, read, len_read, write, len_write);
    std::cout << "Return value of vaccel_genop: " << ret << std::endl;

    return grpc::Status::OK;
}

grpc::Status ServiceImpl::ImageClassification(
    ::grpc::ServerContext *context,
    const ::vaccel::ImageClassificationRequest *request,
    ::vaccel::ImageClassificationResponse *response) {
    printf("Received ImageClassification request\n");

    std::string output_string = "Dummy classification tag";

    response->set_tags(output_string);

    return grpc::Status::OK;
}

grpc::Status
ServiceImpl::CreateResource(::grpc::ServerContext *context,
                            const ::vaccel::CreateResourceRequest *request,
                            ::vaccel::CreateResourceResponse *response) {
    printf("Received CreateResource request with type\n");

    uint32_t dummy_res_id = 12345;
    response->set_resource_id(dummy_res_id);

    return grpc::Status::OK;
}

grpc::Status
ServiceImpl::RegisterResource(::grpc::ServerContext *context,
                              const ::vaccel::RegisterResourceRequest *request,
                              ::vaccel::VaccelEmpty *response) {
    printf("Received RegisterResource request with resource ID: %lu and "
           "session ID: %u\n",
           request->resource_id(),
           static_cast<unsigned int>(request->session_id()));

    return grpc::Status::OK;
}

grpc::Status ServiceImpl::UnregisterResource(
    ::grpc::ServerContext *context,
    const ::vaccel::UnregisterResourceRequest *request,
    ::vaccel::VaccelEmpty *response) {
    printf("Received UnregisterResource request with resource ID: %u and "
           "session ID: %u\n",
           static_cast<unsigned int>(request->resource_id()),
           static_cast<unsigned int>(request->session_id()));

    return grpc::Status::OK;
}

grpc::Status
ServiceImpl::DestroyResource(::grpc::ServerContext *context,
                             const ::vaccel::DestroyResourceRequest *request,
                             ::vaccel::VaccelEmpty *response) {
    printf("Received DestroyResource request with resource ID: %ld\n",
           request->resource_id());

    return grpc::Status::OK;
}

grpc::Status
ServiceImpl::CreateSession(::grpc::ServerContext *context,
                           const ::vaccel::CreateSessionRequest *request,
                           ::vaccel::CreateSessionResponse *response) {
    printf("Received CreateSession request with flag: %d\n", request->flags());

    vaccel_session sess;
    int ret = vaccel_sess_init(&sess, request->flags());
    if (ret != 0) {
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Failed to initialize session");
    }

    uint32_t session_id = sess.session_id;
    response->set_session_id(session_id);

    sessions_map[session_id] = sess;

    return grpc::Status::OK;
}

grpc::Status
ServiceImpl::UpdateSession(::grpc::ServerContext *context,
                           const ::vaccel::UpdateSessionRequest *request,
                           ::vaccel::VaccelEmpty *response) {
    printf("Received UpdateSession request with flag: %d\n", request->flags());

    uint32_t session_id = request->session_id();

    auto it = sessions_map.find(session_id);
    if (it == sessions_map.end()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND,
                            "Session ID not found");
    }

    vaccel_session *sess_ptr = &(it->second);

    int ret = vaccel_sess_update(sess_ptr, request->flags());
    if (ret != 0) {
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Failed to update session");
    }

    return grpc::Status::OK;
}

grpc::Status
ServiceImpl::DestroySession(::grpc::ServerContext *context,
                            const ::vaccel::DestroySessionRequest *request,
                            ::vaccel::VaccelEmpty *response) {
    printf("Received DestroySession request with session ID: %d\n",
           request->session_id());

    uint32_t session_id = request->session_id();

    auto it = sessions_map.find(session_id);
    if (it == sessions_map.end()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND,
                            "Session ID not found");
    }

    vaccel_session *sess_ptr = &(it->second);

    int ret = vaccel_sess_free(sess_ptr);
    if (ret != VACCEL_OK) {
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Failed to destroy session");
    }

    sessions_map.erase(it);

    return grpc::Status::OK;
}

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

std::vector<float> softmax(const float *data, size_t size) {
    std::vector<float> softmaxed_data(size);
    float max_val = *std::max_element(data, data + size);
    float sum = 0.0f;
    for (size_t i = 0; i < size; ++i) {
        softmaxed_data[i] = std::exp(data[i] - max_val);
        sum += softmaxed_data[i];
    }

    for (size_t i = 0; i < size; ++i)
        softmaxed_data[i] /= sum;

    return softmaxed_data;
}

grpc::Status ServiceImpl::TorchJitloadForward(
    ::grpc::ServerContext *context,
    const ::vaccel::TorchJitloadForwardRequest *request,
    ::vaccel::TorchJitloadForwardResponse *response) {

    printf("Received TorchJitloadForward request with model ID: %ld\n",
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
    struct vaccel_torch_saved_model model;
    struct vaccel_torch_buffer run_options = {0};
    int ret;

    std::string run_options_str = "resnet";
    run_options.data = (char *)malloc(run_options_str.size() + 1);
    run_options.size = run_options_str.size() + 1;
    strcpy(run_options.data, run_options_str.c_str());

    const char *model_path = "/home/jl/vaccel-torch-cv-example/";

    vaccel_debug("--------- Imitating previous functions here ---------");
    ret = vaccel_torch_saved_model_set_path(&model, model_path);

    if (ret) {
        vaccel_debug("Could not set model path to Torch model");
        exit(1);
    }

    vaccel_debug("Created new model %lld\n",
                 vaccel_torch_saved_model_id(&model));

    ret = vaccel_torch_saved_model_register(&model);
    if (ret != 0) {
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Failed to register model");
    }

    vaccel_debug("Registered model %lld\n",
                 vaccel_torch_saved_model_id(&model));

    ret = vaccel_sess_register(sess_ptr, model.resource);
    if (ret) {
        fprintf(stderr, "Could not register model with session\n");
        exit(1);
    }

    vaccel_debug("--------- Imitating previous functions here ---------");
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

    // int kTOP_K = 3;

    // printf("Success!\n");
    // printf("Result Tensor :\n");
    // printf("Output tensor => type:%u nr_dims:%u\n", out->data_type,
    //        out->nr_dims);


    // OutputData outf;
    // outf.data = reinterpret_cast<float *>(out->data);
    // outf.size = 1000;

    // std::vector<size_t> indices(outf.size);
    // for (size_t i = 0; i < outf.size; ++i)
    //     indices[i] = i;

    // std::sort(indices.begin(), indices.end(), [&outf](size_t a, size_t b) {
    //     return outf.data[a] > outf.data[b];
    // });

    // std::vector<float> softmaxed_data = softmax(outf.data, outf.size);

    // for (int i = 0; i < kTOP_K; ++i) {
    //     size_t idx = indices[i];
    //     std::cout << "    ============= Top-" << i + 1
    //               << " =============" << std::endl;
    //     std::cout << "    With Probability:  " << softmaxed_data[idx] * 100.0f
    //               << "%" << std::endl;
    // }


    // Encode the output tensor into the gRPC response
    vaccel::TorchJitloadForwardResult* result = response->mutable_torch_result();
    for (int i = 0; i < 1; ++i) { 
        vaccel::TorchTensor* out_tensor = result->add_out_tensors();
        for (int j = 0; j < out[i].nr_dims; ++j) {
            out_tensor->add_dims(out[i].dims[j]);
        }
        out_tensor->set_data(out[i].data, out[i].size);
        out_tensor->set_type(static_cast<vaccel::TorchDataType>(out[i].data_type));
    }

    ret = vaccel_sess_unregister(sess_ptr, model.resource);
    if (ret) {
        fprintf(stderr, "Could not unregister model with session\n");
    }

    vaccel_debug("Completed");

    return grpc::Status::OK;
}

grpc::Status ServiceImpl::TorchLoadModel(
    ::grpc::ServerContext *context,
    const ::vaccel::TorchJitLoadModelFromPathRequest *request,
    ::vaccel::TorchJitLoadModelFromPathResponse *response) {

    const std::string &model_path_bytes = request->model_path();

    printf("Received TorchLoadModel request\n");

    return grpc::Status::OK;
}

grpc::Status ServiceImpl::TorchRegisterModel(
    ::grpc::ServerContext *context,
    const ::vaccel::TorchJitRegisterModelRequest *request,
    ::vaccel::VaccelEmpty *response) {

    int model_id = request->model_id();

    printf("Received TorchRegisterModel request\n");

    return grpc::Status::OK;
}

grpc::Status ServiceImpl::ImagePose(::grpc::ServerContext *context,
                                    const ::vaccel::ImagePoseRequest *request,
                                    ::vaccel::ImagePoseResponse *response) {

    printf("Received ImagePose request\n");

    std::string output_string = "Dummy classification tag";

    response->set_tags(output_string);

    return grpc::Status::OK;
}