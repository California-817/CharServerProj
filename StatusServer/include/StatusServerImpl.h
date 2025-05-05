#pragma once
#include"Const.h"
#include"Calldata.h"
using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using GateServer::Status::GetChatServerReq;
using GateServer::Status::GetChatServerRsp;
using GateServer::Status::StatusServer;

class StatusServerImpl final
{
public:
    StatusServerImpl();
    void Run(uint16_t port);
    // This can be run in multiple threads if needed.
    void HandleRpcs();
    ~StatusServerImpl();
private:
    std::unique_ptr<ServerCompletionQueue> _cq; //完成队列
    StatusServer::AsyncService _service;   //异步grpc服务
    std::unique_ptr<Server> _server; //服务器
    std::vector<std::thread> _threads; //用户层面多线程处理请求
public:
    std::vector<ChatServer> _chat_servers; //所有的chat服务器
    int _server_index; //轮询算法选择chat服务器
};
