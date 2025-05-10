#pragma once
#include"Singleton.h"
#include"Const.h"
//多个io_context对应多个线程 每个线程串行处理多个socket
class IOContextPool:public Singleton<IOContextPool>
{
public:
    IOContextPool();
    std::shared_ptr<boost::asio::io_context>& GetIOContext();
    ~IOContextPool();
private:
    IOContextPool(const IOContextPool&)=delete;
    IOContextPool& operator=(const IOContextPool&)=delete;
    void Stop();
private:
    std::vector<std::shared_ptr<boost::asio::io_context>> _io_contexts; 
    //每一个io_context绑定一个work 可以在run没有事件需要处理的时候不会退出 告诉io_context还在work
    //构造时通知work开始 析构时通知work结束 
    std::vector<std::shared_ptr<boost::asio::io_context::work>> _io_works;
    std::vector<std::thread> _io_threads;
    ssize_t _index;//用于负载均衡派发socket给多个io_context 轮询算法
    ssize_t _size;
};