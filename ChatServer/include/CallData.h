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
using ChatServer::Chat::KickUserReq;
using ChatServer::Chat::KickUserRsp;
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
public:
    NotifyAuthFriendCalldata(ChatService::AsyncService* service, ServerCompletionQueue* cq,ChatServerImpl* impl);
    virtual void Proceed();
    virtual ~NotifyAuthFriendCalldata()=default;
private:
    AuthFriendReq _req;
    AuthFriendRsp _rsp;
    ServerAsyncResponseWriter<AuthFriendRsp> _responder; //写入响应的对象
};
//文本消息通信的grpc处理逻辑
class NotifyTextChatMsgCalldata:public Calldata
{
public:
    NotifyTextChatMsgCalldata(ChatService::AsyncService* service, ServerCompletionQueue* cq,ChatServerImpl* impl);
    virtual void Proceed();
    virtual ~NotifyTextChatMsgCalldata()=default;
private:
    TextChatMsgReq _req;
    TextChatMsgRsp _rsp;
    ServerAsyncResponseWriter<TextChatMsgRsp> _responder; //写入响应的对象
};
//踢人通知的grpc处理逻辑
class NotifyKickUserCalldata:public Calldata
{
public:
    NotifyKickUserCalldata(ChatService::AsyncService* service, ServerCompletionQueue* cq,ChatServerImpl* impl);
    virtual void Proceed();
    virtual ~NotifyKickUserCalldata()=default;
private:
    KickUserReq _req;
    KickUserRsp _rsp;
    ServerAsyncResponseWriter<KickUserRsp> _responder; //写入响应的对象
};

