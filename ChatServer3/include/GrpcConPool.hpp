#pragma once
#include"Const.h"
#include<grpcpp/grpcpp.h>
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
template <class ServerType,class ServerStubType>
class GrpcConPool
{
public:
    GrpcConPool(const std::string& host,const std::string& port,size_t size=std::thread::hardware_concurrency())
    :_host(host),_port(port),_size(size),_b_stop(false)
    {
        for(int i=0;i<_size;i++)
        {
            //grpc的连接复用机制：gRPC默认会复用现有的 TCP 连接，以减少资源消耗和提高性能
            //即使你创建了多个Stub,它们也会共享同一个底层 TCP 连接。这是 gRPC 的正常行为，旨在减少资源消耗和提高性能
            //因此有size个stub,但是底层只有一个tcp连接 netstat -ntp查看只有一个tcp连接
            std::shared_ptr<Channel> channel=grpc::CreateChannel(host+":"+port,grpc::InsecureChannelCredentials());
            //gRPC的Channel对象是线程安全的,并且可以被多个Stub共享
            //这意味着即使你创建了多个Stub,它们也可能共享同一个底层 TCP 连接
            _stubs.push(ServerType::NewStub(channel)); //创建一个与客户端grpc通信的通道
        }
        // 测试池中连接的数量
        std::cout << "Number of stubs in pool: " << _stubs.size() << std::endl;
    }
    std::unique_ptr<ServerStubType> GetGrpcCon()
    {
        std::unique_lock<std::mutex> _ulock(_mtx);
        while(_stubs.empty()&&!_b_stop)  //有链接或者停止的时候退出
        {
            _cv.wait(_ulock);//等待就绪
        }
        if(_b_stop) //因为暂停而出来
        {
            return std::unique_ptr<ServerStubType>(); //返回空智能指针
        }//有链接
        auto con=std::move(_stubs.front());
        _stubs.pop();
        return con;
    }
    void ReturnGrpcCon(std::unique_ptr<ServerStubType> con)
    {
        std::lock_guard<std::mutex> _ulock(_mtx);
        if(_b_stop)
        {
            return;}
        _stubs.push(std::move(con));
        _cv.notify_one();
    }
    void Close()
    {
        _b_stop=true;
        _cv.notify_all();//先设置true在唤醒 保证所有的线程都能出等待循环
    }
    ~GrpcConPool()
    {
        std::lock_guard<std::mutex> _ulock(_mtx);
        Close();
        while(!_stubs.empty())
        {
            _stubs.pop();
        }
    }
private:
    std::queue<std::unique_ptr<ServerStubType>> _stubs;
    std::mutex _mtx;
    std::condition_variable _cv;
    size_t _size;
    std::atomic<bool> _b_stop;
    std::string _host;
    std::string _port;
};