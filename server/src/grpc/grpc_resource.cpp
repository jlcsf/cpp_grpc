#include "service_registry.h"
#include <cstddef>
#include <cstdint>
#include <log.h>
#include <opencv2/opencv.hpp>
#include <random>
#include <sys/types.h>

#include "vaccel.h"

grpc::Status
ServiceImpl::CreateResource(::grpc::ServerContext *context,
                            const ::vaccel::CreateResourceRequest *request,
                            ::vaccel::CreateResourceResponse *response) {
    printf("Received CreateResource request with type\n");
    vaccel_resource_t res_type;

    if (request->has_tf()) {
        res_type = VACCEL_RES_TF_MODEL;
    } else if (request->has_caffe()) {
        res_type = VACCEL_RES_CAFFE_MODEL;
    } else if (request->has_tf_saved()) {
        res_type = VACCEL_RES_TF_SAVED_MODEL;
    } else if (request->has_shared_obj()) {
        res_type = VACCEL_RES_SHARED_OBJ;
    } else if (request->has_torch_saved()) {
        res_type = VACCEL_RES_TORCH_SAVED_MODEL;
    }

    // void *data = nullptr;
    // int (*cleanup_resource)(void *) = nullptr;

    // int ret = resource_new(res, res_type, data, cleanup_resource);
    // if (ret != 0) {
    //     return grpc::Status(grpc::StatusCode::INTERNAL,
    //                         "Failed to initialize resource");
    // }

    // uint32_t resource_id = res->id;
    // response->set_resource_id(resource_id);

    // resources_map[resource_id] = res;

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

    uint32_t session_id = request->session_id();
    uint32_t resource_id = request->resource_id();

    auto it = sessions_map.find(session_id);
    if (it == sessions_map.end()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND,
                            "Session ID not found");
    }

    vaccel_session *sess_ptr = &(it->second);

    auto itr = resources_map.find(resource_id);
    if (itr == resources_map.end()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND,
                            "resource ID not found");
    }

    vaccel_resource *res_ptr = (itr->second);

    int ret = vaccel_sess_register(sess_ptr, res_ptr);

    if (ret != 0) {
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Failed to register resource");
    }

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

    uint32_t session_id = request->session_id();
    uint32_t resource_id = request->resource_id();

    auto it = sessions_map.find(session_id);
    if (it == sessions_map.end()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND,
                            "Session ID not found");
    }

    vaccel_session *sess_ptr = &(it->second);

    auto itr = resources_map.find(resource_id);
    if (itr == resources_map.end()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND,
                            "resource ID not found");
    }

    vaccel_resource *res_ptr = (itr->second);

    int ret = vaccel_sess_unregister(sess_ptr, res_ptr);

    if (ret != 0) {
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Failed to unregister resource");
    }

    return grpc::Status::OK;
}

grpc::Status
ServiceImpl::DestroyResource(::grpc::ServerContext *context,
                             const ::vaccel::DestroyResourceRequest *request,
                             ::vaccel::VaccelEmpty *response) {
    printf("Received DestroyResource request with resource ID: %ld\n",
           request->resource_id());

    uint32_t resource_id = request->resource_id();

    auto itr = resources_map.find(resource_id);
    if (itr == resources_map.end()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND,
                            "resource ID not found");
    }

    vaccel_resource *res_ptr = (itr->second);

    // int ret = resource_destroy(res_ptr);

    // if (ret != 0) {
    //     return grpc::Status(grpc::StatusCode::INTERNAL,
    //                         "Failed to destroy resource");
    // }

    return grpc::Status::OK;
}