#pragma once
#include"Const.h"
#include"UserMgr.h"
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
class ChatServerImpl;
class Calldata
{
public:
    Calldata(ChatService::AsyncService* service,ServerCompletionQueue* cq,ChatServerImpl* impl);
    // Pure virtual function to be implemented by derived classes
    virtual void Proceed()=0; //纯虚函数
    virtual ~Calldata()=default;
protected:
    ChatService::AsyncService * _service;
    ServerCompletionQueue* _cq;
    ServerContext _ctx;
    enum CallStatus {CREATE,PROCESS,FINISH};
    CallStatus _status; // The current serving state.
    ChatServerImpl* _impl;
};

//添加好友的grpc处理逻辑
class NotifyAddFriendCalldata:public Calldata
{
public:
    NotifyAddFriendCalldata(ChatService::AsyncService* service, ServerCompletionQueue* cq,ChatServerImpl* impl);
    virtual void Proceed();
    virtual ~NotifyAddFriendCalldata()=default;
private:
    AddFriendReq _req;
    AddFriendRsp _rsp;
    ServerAsyncResponseWriter<AddFriendRsp> _responder; //写入响应的对象
};
//认证好友的grpc处理逻辑
class NotifyAuthFriendCalldata:public Calldata
{

};
//文本消息通信的grpc处理逻辑
class NotifyTextChatMsgCalldata:public Calldata
{

};
