#pragma once
#include"Const.h"
#include"RedisMgr.h"
#include"DistLock.h"
using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using GateServer::Status::GetChatServerReq;
using GateServer::Status::GetChatServerRsp;
using GateServer::Status::StatusServer;
using GateServer::Status::LoginReq;
using GateServer::Status::LoginRsp;
class StatusServerImpl;
class Calldata
{
public:
    Calldata(StatusServer::AsyncService* service,ServerCompletionQueue* cq,StatusServerImpl* impl);
    // Pure virtual function to be implemented by derived classes
    virtual void Proceed()=0; //纯虚函数
    virtual ~Calldata()=default;
protected:
    StatusServer::AsyncService* _service;
    ServerCompletionQueue* _cq;
    ServerContext _ctx;
    enum CallStatus {CREATE,PROCESS,FINISH};
    CallStatus _status; // The current serving state.
    StatusServerImpl* _impl;
};
//GateServer来获取ChatServer信息的请求
class GetChatServerCalldata: public Calldata
{
public:
    GetChatServerCalldata(StatusServer::AsyncService* service, ServerCompletionQueue* cq,StatusServerImpl* impl);
    std::string generate_unique_string();
    ChatServer_struct getChatServer();
    void insertToken(int uid,std::string token);
    virtual void Proceed();
    virtual ~GetChatServerCalldata()=default;
private:
    GetChatServerReq _req;  //获取chatserver的请求
    GetChatServerRsp _rsp;  //获取chatserver的响应
    ServerAsyncResponseWriter<GetChatServerRsp> _responder; //写入响应的对象
};
//ChatServer来进行登录验证的请求
class LoginCalldata:public Calldata
{
public:
    LoginCalldata(StatusServer::AsyncService* service, ServerCompletionQueue* cq,StatusServerImpl* impl);
    virtual void Proceed();
    virtual ~LoginCalldata()=default;
private:
    LoginReq _req;
    LoginRsp _rsp;
    ServerAsyncResponseWriter<LoginRsp> _responder; //写入响应的对象
};
