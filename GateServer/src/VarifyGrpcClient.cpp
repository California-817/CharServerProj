#include"../include/VarifyGrpcClient.h"

GrpcConPool::GrpcConPool(const std::string& host,const std::string& port,size_t size)
:_size(size),_b_stop(false),_host(host),_port(port)
{
    for(int i=0;i<_size;i++)
    {
        std::shared_ptr<Channel> channel=grpc::CreateChannel(host+":"+port,grpc::InsecureChannelCredentials());
        _stubs.push(VarifyServer::NewStub(channel)); //创建一个与客户端grpc通信的通道
    }
}
std::unique_ptr<VarifyServer::Stub> GrpcConPool::GetGrpcCon() //unique_ptr只有移动构造且不能引用 传值返回并且值接收 会执行一次移动构造
{
    std::unique_lock<std::mutex> _ulock(_mtx);
    while(_stubs.empty()&&!_b_stop)  //有链接或者停止的时候退出
    {
        _cv.wait(_ulock);//等待就绪
    }
    if(_b_stop) //因为暂停而出来
    {
        return std::unique_ptr<VarifyServer::Stub>(); //返回空智能指针
    }//有链接
    auto con=std::move(_stubs.front());
    _stubs.pop();
    return con;
}
void GrpcConPool::ReturnGrpcCon(std::unique_ptr<VarifyServer::Stub> con)
{   
    std::lock_guard<std::mutex> _ulock(_mtx);
    if(_b_stop)
    {
        return;}
    _stubs.push(std::move(con));
    _cv.notify_one();
}
void GrpcConPool::Close()
{       
    _b_stop=true;
    _cv.notify_all();//先设置true在唤醒 保证所有的线程都能出等待循环
}   
GrpcConPool::~GrpcConPool()
{
    std::lock_guard<std::mutex> _ulock(_mtx);
    Close();
    while(!_stubs.empty())
    {
        _stubs.pop();
    }
}

VarifyGrpcClient::VarifyGrpcClient()
{
    //初始化pool
    mINI::INIFile file("../conf/config.ini");
    mINI::INIStructure ini;
    file.read(ini);
    std::string host=ini["VarifyServer"]["host"];
    std::string port=ini["VarifyServer"]["port"];
    std::string size=ini["GrpcPool"]["size"];
    _grpc_con_pool.reset(new GrpcConPool(host,port,atoi(size.c_str())));
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