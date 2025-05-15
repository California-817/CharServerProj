#pragma once
#include"GrpcConPool.hpp"
#include"grpc_cpp_out/GateServer.Status.grpc.pb.h"
#include"grpc_cpp_out/GateServer.Status.pb.h"
#include"Singleton.h"
#include<grpcpp/grpcpp.h>
#include"Const.h"
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using GateServer::Status::LoginReq;
using GateServer::Status::LoginRsp;
using GateServer::Status::StatusServer;
class LoginGrpcClient: public Singleton<LoginGrpcClient>
{
public:
    friend class Singleton<LoginGrpcClient>;
    LoginGrpcClient();
    LoginRsp Login(int uid,std::string token);
    ~LoginGrpcClient()=default;
private:
    LoginGrpcClient(const LoginGrpcClient&)=delete;
    LoginGrpcClient& operator=(const LoginGrpcClient&)=delete;
private:
    std::unique_ptr<GrpcConPool<StatusServer,StatusServer::Stub>> _pool;
};

