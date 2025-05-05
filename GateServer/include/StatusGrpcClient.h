#pragma once
#include"grpc_cpp_out/GateServer.Status.pb.h"
#include"grpc_cpp_out/GateServer.Status.grpc.pb.h"
#include"Const.h"
#include"Singleton.h"
#include<grpcpp/grpcpp.h>
#include"GrpcConPool.hpp"
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using GateServer::Status::GetChatServerReq;
using GateServer::Status::GetChatServerRsp;
using GateServer::Status::StatusServer;

class StatusGrpcClient:public Singleton<StatusGrpcClient>
{
public:
    friend class Singleton<StatusGrpcClient>;
    GetChatServerRsp GetChatServer(int uid);
    ~StatusGrpcClient()=default;
private:
    StatusGrpcClient();
    StatusGrpcClient(const StatusGrpcClient&)=delete;
    StatusGrpcClient& operator=(const StatusGrpcClient&)=delete;
private:
    std::unique_ptr<GrpcConPool<StatusServer,StatusServer::Stub>> _pool;
};