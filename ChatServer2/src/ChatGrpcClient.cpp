#include"../include/ChatGrpcClient.h"
ChatGrpcClient::ChatGrpcClient()
{ //每一个对端服务器都生成一个grpcpool进行连接
    mINI::INIFile file("../conf/config.ini");
    mINI::INIStructure ini;
    file.read(ini);
    std::string section="PeerServers";
    std::string key="servers";
    std::string value=ini[section][key];
    // 分割字段值
    std::vector<std::string> values;
    std::stringstream ss(value);
    std::string item;
    while (std::getline(ss, item, ',')) {
        values.push_back(item);}
    for(auto& e: values)
    {
        _pools.insert(std::make_pair(ini[e]["name"],std::make_shared<GrpcConPool<ChatService,ChatService::Stub>>(
                        ini[e]["host"],ini[e]["rpcport"],atoi(ini["GrpcPool"]["size"].c_str()))));
    }
}
//添加好友
AddFriendRsp ChatGrpcClient::NotifyAddFriend(const std::string& ip,const AddFriendReq& req)
{
    //1.根据ip找到指定的pool
    auto it=_pools.find(ip);
    if(it==_pools.end())
    { //没找到这个ip对应的pool
        return AddFriendRsp();}
    auto grpc_pool=it->second;
    AddFriendRsp rsp;
    ClientContext clicontext;
    //the actual grpc 客户端同步调用grpc
    auto stub=grpc_pool->GetGrpcCon(); //从grpc连接池中获取连接
    Defer defer([&stub,&grpc_pool](){
        grpc_pool->ReturnGrpcCon(std::move(stub)); //归还连接
    });
    Status status=stub->NotifyAddFriend(&clicontext,req,&rsp);
    if(status.ok())
    { //调用正常
        return rsp;
    }else{
        std::cout << status.error_code() << ": " << status.error_message()
        << std::endl;
        rsp.set_error(ErrorCodes::RPCFailed); //调用失败 自己这端给响应设置错误值
        return rsp;
    }
    return rsp;
}
//认证好友
AuthFriendRsp ChatGrpcClient::NotifyAuthFriend(const std::string& ip,const AuthFriendReq& req)
{
    //1.根据ip找到指定的pool
    auto it=_pools.find(ip);
    if(it==_pools.end())
    { //没找到这个ip对应的pool
        return AuthFriendRsp();}
    auto grpc_pool=it->second;
    AuthFriendRsp rsp;
    ClientContext clicontext;
    //the actual grpc 客户端同步调用grpc
    auto stub=grpc_pool->GetGrpcCon(); //从grpc连接池中获取连接
    Defer defer([&stub,&grpc_pool](){
        grpc_pool->ReturnGrpcCon(std::move(stub)); //归还连接
    });
    Status status=stub->NotifyAuthFriend(&clicontext,req,&rsp);
    if(status.ok())
    { //调用正常
        return rsp;
    }else{
        std::cout << status.error_code() << ": " << status.error_message()
        << std::endl;
        rsp.set_error(ErrorCodes::RPCFailed); //调用失败 自己这端给响应设置错误值
        return rsp;
    }
    return rsp;   
}
//文本消息通信
TextChatMsgRsp ChatGrpcClient::NotifyTextChatMsg(const std::string& ip,const TextChatMsgReq& req)
{
    //1.根据ip找到指定的pool
    auto it=_pools.find(ip);
    if(it==_pools.end())
    { //没找到这个ip对应的pool
        return TextChatMsgRsp();}
    auto grpc_pool=it->second;
    TextChatMsgRsp rsp;
    ClientContext clicontext;
    //the actual grpc 客户端同步调用grpc
    auto stub=grpc_pool->GetGrpcCon(); //从grpc连接池中获取连接
    Defer defer([&stub,&grpc_pool](){
        grpc_pool->ReturnGrpcCon(std::move(stub)); //归还连接
    });
    Status status=stub->NotifyTextChatMsg(&clicontext,req,&rsp);
    if(status.ok())
    { //调用正常
        return rsp;
    }else{
        std::cout << status.error_code() << ": " << status.error_message()
        << std::endl;
        rsp.set_error(ErrorCodes::RPCFailed); //调用失败 自己这端给响应设置错误值
        return rsp;
    }
    return rsp;      
}
//踢人通知
KickUserRsp ChatGrpcClient::NotifyKickUser(const std::string& ip,const KickUserReq& req)
{
    //1.根据ip找到指定的pool
    auto it=_pools.find(ip);
    if(it==_pools.end())
    { //没找到这个ip对应的pool
        return KickUserRsp();}
    auto grpc_pool=it->second;
    KickUserRsp rsp;
    ClientContext clicontext;
    //the actual grpc 客户端同步调用grpc
    auto stub=grpc_pool->GetGrpcCon(); //从grpc连接池中获取连接
    Defer defer([&stub,&grpc_pool](){
        grpc_pool->ReturnGrpcCon(std::move(stub)); //归还连接
    });
    Status status=stub->NotifyKickUser(&clicontext,req,&rsp);
    if(status.ok())
    { //调用正常
        return rsp;
    }else{
        std::cout << status.error_code() << ": " << status.error_message()
        << std::endl;
        rsp.set_error(ErrorCodes::RPCFailed); //调用失败 自己这端给响应设置错误值
        return rsp;
    }
    return rsp;   
}
ChatGrpcClient::~ChatGrpcClient()
{
    _pools.clear();
}

