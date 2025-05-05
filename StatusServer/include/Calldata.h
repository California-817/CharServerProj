#pragma once
#include"Const.h"
using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using GateServer::Status::GetChatServerReq;
using GateServer::Status::GetChatServerRsp;
using GateServer::Status::StatusServer;
class StatusServerImpl;
class Calldata
{
public:
    Calldata(StatusServer::AsyncService* service,ServerCompletionQueue* cq,StatusServerImpl* impl);
    // Pure virtual function to be implemented by derived classes
    virtual void Proceed()=0;
    virtual ~Calldata()=default;
protected:
    StatusServer::AsyncService* _service;
    ServerCompletionQueue* _cq;
    ServerContext _ctx;
    enum CallStatus {CREATE,PROCESS,FINISH};
    CallStatus _status; // The current serving state.
    StatusServerImpl* _impl;
};
class GetChatServerCalldata: public Calldata
{
public:
    GetChatServerCalldata(StatusServer::AsyncService* service, ServerCompletionQueue* cq,StatusServerImpl* impl);
    virtual void Proceed();
    virtual ~GetChatServerCalldata()=default;
private:
    GetChatServerReq _req;  //获取chatserver的请求
    GetChatServerRsp _rsp;  //获取chatserver的响应
    ServerAsyncResponseWriter<GetChatServerRsp> _responder; //写入响应的对象
};
