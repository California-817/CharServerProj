#include"../include/VarifyGrpcClient.h"

VarifyGrpcClient::VarifyGrpcClient()
{
    //初始化pool
    mINI::INIFile file("../conf/config.ini");
    mINI::INIStructure ini;
    file.read(ini);
    std::string host=ini["VarifyServer"]["host"];
    std::string port=ini["VarifyServer"]["port"];
    std::string size=ini["GrpcPool"]["size"];
    std::cout<<size<<std::endl;
    _grpc_con_pool.reset(new GrpcConPool<VarifyServer,VarifyServer::Stub>(host,port,atoi(size.c_str())));
}
GetVarifyRsp VarifyGrpcClient::GetVarify(const std::string& email)
{
    GetVarifyReq req;
    req.set_email(email);
    GetVarifyRsp rsp;
    ClientContext clicontext;
    //the actual grpc 客户端同步调用grpc
    auto stub=_grpc_con_pool->GetGrpcCon(); //从grpc连接池中获取连接
    Status status=stub->GetVarifyCode(&clicontext,req,&rsp);
    if(status.ok())
    { //调用正常
        _grpc_con_pool->ReturnGrpcCon(std::move(stub)); //归还连接
        return rsp;
    }else{
        _grpc_con_pool->ReturnGrpcCon(std::move(stub));
        std::cout << status.error_code() << ": " << status.error_message()
        << std::endl;
        rsp.set_error(ErrorCodes::RPCFailed); //调用失败 自己这端给响应设置错误值
        return rsp;
    }
    return rsp;
}