#pragma once
#include"Const.h"
#include"Singleton.h"
class IOServerPool : public Singleton<IOServerPool>
{
public:
    friend class Singleton<IOServerPool>;
    boost::asio::io_context& GetIOService();
    void ResetIndex(); //当创建一个socket失败的时候回退到上一个io_context
    ~IOServerPool();
private:
    IOServerPool(size_t pool_size=std::thread::hardware_concurrency());
    IOServerPool(const IOServerPool&)=delete;
    IOServerPool& operator=(const IOServerPool&)=delete;
private:
    std::vector<boost::asio::io_context> _io_contexts;
    std::vector<std::unique_ptr<boost::asio::io_context::work>> _works; //防止无事件注册导致退出
    std::vector<std::thread> _threads;
    size_t _pool_size;
    size_t _index; //轮询获取iocontext
};