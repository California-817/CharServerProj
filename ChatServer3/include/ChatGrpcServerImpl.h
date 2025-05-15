#pragma once
#include"Const.h"
#include"CallData.h"
//grpc服务端
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using ChatServer::Chat::AddFriendReq;
using ChatServer::Chat::AddFriendRsp;
using ChatServer::Chat::AuthFriendReq;
using ChatServer::Chat::AuthFriendRsp;
using ChatServer::Chat::TextChatMsgReq;
using ChatServer::Chat::TextChatMsgRsp;
using ChatServer::Chat::ChatService;
class ChatServerImpl final 
{
public:
    ChatServerImpl();
    void Run(uint16_t port);
    // This can be run in multiple threads if needed.
    void HandleRpcs();
    ~ChatServerImpl();
private:
    ChatService::AsyncService _service;   //异步grpc服务
    std::vector<std::thread> _threads; //用户层面多线程处理请求
public:
    std::unique_ptr<grpc::Server> _server; //服务器
    std::unique_ptr<ServerCompletionQueue> _cq; //完成队列

};

