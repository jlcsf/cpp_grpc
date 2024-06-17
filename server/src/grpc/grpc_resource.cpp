#include "service_registry.h"
#include <cstddef>
#include <cstdint>
#include <log.h>
#include <opencv2/opencv.hpp>
#include <random>
#include <sys/types.h>

#include <vaccel.h>

/// here if I include resource.h in src it builds fine

#include <stdint.h>
#include <stdatomic.h>
extern "C"
{
    struct vaccel_resource {
	/* resource id */
	vaccel_id_t id;

	/* type of the resource */
	vaccel_resource_t type;

	/* type-specific data of the resource */
	void *data;

	/* type-specific destructor */
	int (*cleanup_resource)(void *data);

	/* An entry to add this resource in a list */
	list_entry_t entry;

	/* Reference counter representing the number of sessions
	 * to which this resource is registered to. */
	atomic_uint refcount;

	/* rundir for this resource if it needs it. It can be empty (NULL) */
	char *rundir;
};

int resources_bootstrap(void);
int resources_cleanup(void);
int resource_new(struct vaccel_resource *res, vaccel_resource_t type, void *data,
		int (*cleanup_resource)(void *));
int resource_get_by_id (struct vaccel_resource **resource, vaccel_id_t id);
int resource_destroy(struct vaccel_resource *res);
void resource_refcount_inc(struct vaccel_resource *res);
void resource_refcount_dec(struct vaccel_resource *res);
int resource_create_rundir(struct vaccel_resource *res);
int resource_destroy_rundir(struct vaccel_resource *res);
}

/// here if I include resource.h in src it builds fine


grpc::Status
ServiceImpl::CreateResource(::grpc::ServerContext *context,
                            const ::vaccel::CreateResourceRequest *request,
                            ::vaccel::CreateResourceResponse *response) {
    printf("Received CreateResource request with type\n");
    vaccel_resource_t res_type;

    if (request->has_tf()) {
        res_type = VACCEL_RES_SINGLE_MODEL;
    } else if (request->has_tf_saved()) {
        res_type = VACCEL_RES_TF_SAVED_MODEL;
    } else if (request->has_shared_obj()) {
        res_type = VACCEL_RES_SHARED_OBJ;
    } else if (request->has_torch_saved()) {
        res_type = VACCEL_RES_SINGLE_MODEL;
    }

    struct vaccel_resource res;

    void *data = nullptr;
    int (*cleanup_resource)(void *) = nullptr;

    int ret = resource_new(&res, res_type, data, cleanup_resource);
    if (ret != 0) {
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Failed to initialize resource");
    }

    uint32_t resource_id = res.id; 
    response->set_resource_id(resource_id);

    resources_map[resource_id] = &res;

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