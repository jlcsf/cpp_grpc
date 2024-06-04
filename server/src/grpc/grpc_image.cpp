#include "service_registry.h"
#include <cstddef>
#include <cstdint>
#include <log.h>
#include <opencv2/opencv.hpp>
#include <random>
#include <sys/types.h>

#include "vaccel.h"

grpc::Status ServiceImpl::ImageClassification(
    ::grpc::ServerContext *context,
    const ::vaccel::ImageClassificationRequest *request,
    ::vaccel::ImageClassificationResponse *response) {
    printf("Received ImageClassification request\n");

    std::string output_string = "Dummy classification tag";

    response->set_tags(output_string);

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