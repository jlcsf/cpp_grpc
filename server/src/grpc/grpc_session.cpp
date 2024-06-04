#include "service_registry.h"
#include <cstddef>
#include <cstdint>
#include <log.h>
#include <opencv2/opencv.hpp>
#include <random>
#include <sys/types.h>

#include "vaccel.h"

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