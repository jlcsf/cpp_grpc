#include "service_registry.h"
#include <cstddef>
#include <cstdint>
#include <log.h>
#include <opencv2/opencv.hpp>
#include <random>
#include <sys/types.h>

#include "vaccel.h"

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

    // Encode the result into the gRPC response
    vaccel::GenopResult *result = response->mutable_genop_result();
    for (int i = 0; i < len_write; ++i) {
        vaccel::GenopArg *write_arg = result->add_write_args();
        write_arg->set_argtype(1);
        write_arg->set_size(write[i].size);
        write_arg->set_buf(write[i].buf, write[i].size);
    }

    return grpc::Status::OK;
}