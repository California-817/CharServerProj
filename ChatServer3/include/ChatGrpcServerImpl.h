#pragma once
#include"Const.h"
#include"CallData.h"
#include"RedisMgr.h"
#include"MysqlMgr.h"
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
using ChatServer::Chat::KickUserReq;
using ChatServer::Chat::KickUserRsp;
using ChatServer::Chat::ChatService;
class Server;
class ChatServerImpl final 
{
public:
    ChatServerImpl(Server* p_server);
    void Run(uint16_t port);
    // This can be run in multiple threads if needed.
    void HandleRpcs();
    bool GetUserInfo(int uid,UserInfo& userinfo); //获取用户信息
    ~ChatServerImpl();
private:
    ChatService::AsyncService _service;   //异步grpc服务
    std::vector<std::thread> _threads; //用户层面多线程处理请求
public:
    std::unique_ptr<grpc::Server> _server; //grpc服务器
    std::unique_ptr<ServerCompletionQueue> _cq; //完成队列
    Server* _p_server; //tcp服务器

};

