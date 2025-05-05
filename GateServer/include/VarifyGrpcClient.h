#pragma once
#include"Const.h"
#include<grpcpp/grpcpp.h>
#include"grpc_cpp_out/GateServer.Varify.pb.h"
#include"grpc_cpp_out/GateServer.Varify.grpc.pb.h"
#include"Singleton.h"
#include"GrpcConPool.hpp"
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using GateServer::Varify::VarifyServer;
using GateServer::Varify::GetVarifyReq;
using GateServer::Varify::GetVarifyRsp;

class VarifyGrpcClient: public Singleton<VarifyGrpcClient>
{
public:
    friend class Singleton<VarifyGrpcClient>;
    GetVarifyRsp GetVarify(const std::string& email);
    ~VarifyGrpcClient()=default;
private:
    VarifyGrpcClient();
    VarifyGrpcClient(const VarifyGrpcClient&)=delete;
    VarifyGrpcClient& operator=(const VarifyGrpcClient&)=delete;
private:
    // std::unique_ptr<VarifyServer::Stub> _stub; //该服务的客户端存根
    std::unique_ptr<GrpcConPool<VarifyServer,VarifyServer::Stub>> _grpc_con_pool;
};