#pragma once
#include"Const.h"
#include<grpcpp/grpcpp.h>
#include"GateServer.Varify.pb.h"
#include"GateServer.Varify.grpc.pb.h"
#include"Singleton.h"
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
private:
    std::unique_ptr<VarifyServer::Stub> _stub; //该服务的客户端存根
};