#pragma once
#include"Const.h"
#include<grpcpp/grpcpp.h>
#include"GateServer.Varify.pb.h"
#include"GateServer.Varify.grpc.pb.h"
#include"Singleton.h"
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using GateServer::Varify::VarifyServer;
using GateServer::Varify::GetVarifyReq;
using GateServer::Varify::GetVarifyRsp;
 
class GrpcConPool  //grpc的连接池
{
public:
    GrpcConPool(const std::string& host,const std::string& port,size_t size=std::thread::hardware_concurrency());
    std::unique_ptr<VarifyServer::Stub> GetGrpcCon(); //unique_ptr只有移动构造且不能引用 传值返回并且值接收 会执行一次移动构造
    void ReturnGrpcCon(std::unique_ptr<VarifyServer::Stub> con);
    void Close();
    ~GrpcConPool();
private:
    std::queue<std::unique_ptr<VarifyServer::Stub>> _stubs;
    std::mutex _mtx;
    std::condition_variable _cv;
    size_t _size;
    std::atomic<bool> _b_stop;
    std::string _host;
    std::string _port;
};
class VarifyGrpcClient: public Singleton<VarifyGrpcClient>
{
public:
    friend class Singleton<VarifyGrpcClient>;
    GetVarifyRsp GetVarify(const std::string& email);
    ~VarifyGrpcClient()=default;
private:
    VarifyGrpcClient();
private:
    // std::unique_ptr<VarifyServer::Stub> _stub; //该服务的客户端存根
    std::unique_ptr<GrpcConPool> _grpc_con_pool;
};