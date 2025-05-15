#include "../include/IOContextPool.h"
IOContextPool::IOContextPool() // 默认为cpu的核数
    : _index(0), _io_contexts(std::thread::hardware_concurrency()), _io_works(std::thread::hardware_concurrency()),
      _size(std::thread::hardware_concurrency())
{
    for(int i = 0; i < _size; i++)
    {
        _io_contexts[i]=std::make_shared<boost::asio::io_context>(1);
    }
    for (int e = 0; e < _size; e++)
    {
        _io_works[e] = std::make_shared<boost::asio::io_context::work>(*_io_contexts[e]);
        // 每一个io_context都被告知开始工作
    }
    for (int i = 0; i < _size; i++)
    {
        _io_threads.emplace_back([this, i]()
                                 {
            //每一个线程启动事件循环
            this->_io_contexts[i]->run(); });
    }
}
std::shared_ptr<boost::asio::io_context> &IOContextPool::GetIOContext()
{
    return _io_contexts[(_index++) % _size];
}
void IOContextPool::Stop()
{
    for (auto &e : _io_works)
    {
        e->get_io_context().stop(); // 先设置io_context不再处理事件
        // 清除智能指针调用work的析构函数 对应io_context工作状态结束 无事件会退出
        e.reset();
    }
    for (auto &e : _io_threads)
    {
        e.join(); // 等待线程退出
    }
}
IOContextPool::~IOContextPool()
{
    IOContextPool::Stop();
    std::cout << "IOContextPool Exit" << std::endl;
}