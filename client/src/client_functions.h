#ifndef VACCEL_CLIENT_FUNCTIONS_H
#define VACCEL_CLIENT_FUNCTIONS_H

#include <grpcpp/create_channel.h>
#include <grpcpp/impl/channel_interface.h>
#include <memory>
#include <myproto/agent.grpc.pb.h> //

#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

std::string CreateSession(const std::shared_ptr<grpc::Channel> &channel,
                          grpc::ClientContext &context, int flags);

vaccel::VaccelEmpty UpdateSession(const std::shared_ptr<grpc::Channel> &channel,
                                  grpc::ClientContext &context, int session_id,
                                  int flags);

vaccel::VaccelEmpty
DestroySession(const std::shared_ptr<grpc::Channel> &channel,
               grpc::ClientContext &context, int session_id);

std::string CreateResource(const std::shared_ptr<grpc::Channel> &channel,
                           grpc::ClientContext &context,
                           const vaccel::CreateResourceRequest &model);

vaccel::VaccelEmpty
DestroyResource(const std::shared_ptr<grpc::Channel> &channel,
                grpc::ClientContext &context, int resource_id);

vaccel::VaccelEmpty
RegisterResource(const std::shared_ptr<grpc::Channel> &channel,
                 grpc::ClientContext &context, int resource_id, int session_id);

vaccel::VaccelEmpty
UnregisterResource(const std::shared_ptr<grpc::Channel> &channel,
                   grpc::ClientContext &context, int resource_id,
                   int session_id);

std::vector<std::string>
ImageClassification(const std::shared_ptr<grpc::Channel> &channel,
                    grpc::ClientContext &context, int session_id,
                    const std::string &image);


vaccel::GenopResponse Genop(const std::shared_ptr<grpc::Channel> &channel,
                            grpc::ClientContext &context, int session_id,
                            const std::vector<vaccel::GenopArg> &read_args,
                            const std::vector<vaccel::GenopArg> &write_args);

vaccel::TensorflowModelLoadResponse
TensorflowModelLoad(const std::shared_ptr<grpc::Channel> &channel,
                    grpc::ClientContext &context, int session_id, int model_id);

vaccel::TensorflowModelUnloadResponse
TensorflowModelUnload(const std::shared_ptr<grpc::Channel> &channel,
                      grpc::ClientContext &context, int session_id,
                      int model_id);

vaccel::TensorflowModelRunResponse
TensorflowModelRun(const std::shared_ptr<grpc::Channel> &channel,
                   grpc::ClientContext &context, int session_id, int model_id,
                   const std::string &run_options,
                   const std::vector<std::pair<std::string, int>> &in_nodes,
                   const std::vector<std::pair<std::string, int>> &out_nodes,
                   const std::vector<uint8_t> &in_tensors_data);

#endif // VACCEL_CLIENT_FUNCTIONS_H
