#include "../include/LoginGrpcClient.h"
LoginGrpcClient::LoginGrpcClient()
{
    // 初始化pool
    mINI::INIFile file("../conf/config.ini");
    mINI::INIStructure ini;
    file.read(ini);
    std::string host = ini["StatusServer"]["host"];
    std::string port = ini["StatusServer"]["port"];
    std::string size = ini["GrpcPool"]["size"];
    std::cout << size << std::endl;
    _pool.reset(new GrpcConPool<StatusServer, StatusServer::Stub>(host, port, atoi(size.c_str())));
}
LoginRsp LoginGrpcClient::Login(int uid,std::string token)
{
    LoginReq req;
    req.set_uid(uid);
    req.set_token(token);
    LoginRsp rsp;
    ClientContext clicontext;
    //the actual grpc 客户端同步调用grpc
    auto stub=_pool->GetGrpcCon(); //从grpc连接池中获取连接
    Defer defer([&stub,this](){
        this->_pool->ReturnGrpcCon(std::move(stub)); //归还连接
    });
    Status status=stub->Login(&clicontext,req,&rsp);
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