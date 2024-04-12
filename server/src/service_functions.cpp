#include "service_registry.h"
#include "vaccel.h"
#include <ops/vaccel_ops.h>
#include <cstdint>
#include <session.h>

grpc::Status ServiceImpl::Genop(::grpc::ServerContext *context,
                                const ::vaccel::GenopRequest *request,
                                ::vaccel::GenopResponse *response) {
    printf("Received Genop request with session ID: %d\n", request->session_id());

    int len_read = 0;
    int len_write = 0;

    // Retrieve read arguments
    printf("Read arguments:\n");
    for (const auto& arg : request->read_args()) {
        printf("  Argument type: %d, Size: %d\n", arg.argtype(), arg.size());
        len_read += 1;
    }

    // Retrieve write arguments
    printf("Write arguments:\n");
    for (const auto& arg : request->write_args()) {
        printf("  Argument type: %d, Size: %d\n", arg.argtype(), arg.size());
        len_write += 1;
    }

    printf("Total length of read arguments: %d\n", len_read);
    printf("Total length of write arguments: %d\n", len_write);


    uint32_t session_id = request->session_id();

    auto it = sessions_map.find(session_id);
    if (it == sessions_map.end()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Session ID not found");
    }

    vaccel_session *sess_ptr = &(it->second);

    struct vaccel_arg read[len_read];
    struct vaccel_arg write[len_write];
    enum vaccel_op_type op_type;

    const vaccel::GenopArg& genop_arg = request->read_args(0);
    const std::string& byte_sequence = genop_arg.buf();

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
        const std::string& arg_buf = request->read_args(i).buf();
        read[i].size = arg_buf.size();
        read[i].buf = const_cast<char*>(arg_buf.data());
    }

    for (size_t i = 0; i < len_write; ++i) {
        const std::string& arg_buf = request->write_args(i).buf();
        write[i].size = arg_buf.size();
        write[i].buf = const_cast<char*>(arg_buf.data());
    }


    int ret;
    ret = vaccel_genop(sess_ptr, read, len_read, write, len_write);
    std::cout << "Return value of vaccel_genop: " << ret << std::endl;


    return grpc::Status::OK;
}

grpc::Status ServiceImpl::ImageClassification(::grpc::ServerContext *context,
                                               const ::vaccel::ImageClassificationRequest *request,
                                               ::vaccel::ImageClassificationResponse *response) {
    printf("Received ImageClassification request\n");

    std::string output_string = "Dummy classification tag";

    response->set_tags(output_string);

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

    vaccel_session sess;
    int ret = vaccel_sess_init(&sess, request->flags());
    if (ret != 0) {
        return grpc::Status(grpc::StatusCode::INTERNAL, "Failed to initialize session");
    }

    uint32_t session_id = sess.session_id;
    response->set_session_id(session_id);

    sessions_map[session_id] = sess;

    return grpc::Status::OK;
}

grpc::Status ServiceImpl::UpdateSession(::grpc::ServerContext *context,
                                        const ::vaccel::UpdateSessionRequest *request,
                                        ::vaccel::VaccelEmpty *response) {
    printf("Received UpdateSession request with flag: %d\n", request->flags());

    uint32_t session_id = request->session_id();

    auto it = sessions_map.find(session_id);
    if (it == sessions_map.end()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Session ID not found");
    }

    vaccel_session *sess_ptr = &(it->second);

    int ret = vaccel_sess_update(sess_ptr, request->flags());
    if (ret != 0) {
        return grpc::Status(grpc::StatusCode::INTERNAL, "Failed to update session");
    }

    return grpc::Status::OK;
}

grpc::Status ServiceImpl::DestroySession(::grpc::ServerContext *context,
                                         const ::vaccel::DestroySessionRequest *request,
                                         ::vaccel::VaccelEmpty *response) {
    printf("Received DestroySession request with session ID: %d\n", request->session_id());

    uint32_t session_id = request->session_id();

    auto it = sessions_map.find(session_id);
    if (it == sessions_map.end()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Session ID not found");
    }

    vaccel_session *sess_ptr = &(it->second);

    int ret = vaccel_sess_free(sess_ptr);
    if (ret != VACCEL_OK) {
        return grpc::Status(grpc::StatusCode::INTERNAL, "Failed to destroy session");
    }

    sessions_map.erase(it);

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

grpc::Status ServiceImpl::TorchJitloadForward(::grpc::ServerContext *context,
                                     const ::vaccel::TorchJitloadForwardRequest *request,
                                     ::vaccel::TorchJitloadForwardResponse *response) {

    printf("Received TensorflowModelRun request with model ID: %ld\n", request->model_id());

    return grpc::Status::OK;

}
