#include"../include/VarifyGrpcClient.h"
VarifyGrpcClient::VarifyGrpcClient()
{
    std::shared_ptr<Channel> channel=grpc::CreateChannel("localhost:8081",grpc::InsecureChannelCredentials());
    _stub=VarifyServer::NewStub(channel); //创建一个与客户端grpc通信的通道
}
GetVarifyRsp VarifyGrpcClient::GetVarify(const std::string& email)
{
    GetVarifyReq req;
    req.set_email(email);
    GetVarifyRsp rsp;
    ClientContext clicontext;
    //the actual grpc 客户端同步调用grpc
    Status status=_stub->GetVarifyCode(&clicontext,req,&rsp);
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