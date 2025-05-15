#pragma once
#include"Const.h"
#include"Singleton.h"
#include"GrpcConPool.hpp"
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using ChatServer::Chat::AddFriendReq;
using ChatServer::Chat::AddFriendRsp;
using ChatServer::Chat::AuthFriendReq;
using ChatServer::Chat::AuthFriendRsp;
using ChatServer::Chat::TextChatMsgReq;
using ChatServer::Chat::TextChatMsgRsp;
using ChatServer::Chat::ChatService;
class ChatGrpcClient :public Singleton<ChatGrpcClient>
{
public:
    friend class Singleton<ChatGrpcClient>;
    AddFriendRsp NotifyAddFriend(const std::string& ip,const AddFriendReq& req);
    AuthFriendRsp NotifyAuthFriend(const std::string& ip,const AuthFriendReq& req);
    TextChatMsgRsp NotifyTextChatMsg(const std::string& ip,const TextChatMsgReq& req);
    ~ChatGrpcClient();
private:
    ChatGrpcClient();
    ChatGrpcClient(const ChatGrpcClient &)=delete;
    ChatGrpcClient& operator=(const ChatGrpcClient &)=delete;
private:
    std::unordered_map<std::string,std::shared_ptr<GrpcConPool<ChatService,ChatService::Stub>>> _pools; //对端服务器名称以及连接池的映射关系
};
