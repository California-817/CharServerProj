#include"../include/IOServicePool.h"
IOServerPool::IOServerPool(size_t pool_size)
:_pool_size(pool_size),_io_contexts(pool_size),_works(pool_size),_threads(pool_size),_index(0)
{
    for(int i=0;i<_pool_size; i++)
    { //work与io_context绑定
        _works[i]=std::make_unique<boost::asio::io_context::work>(_io_contexts[i]);
    }
    for(int i=0;i<_pool_size;i++)
    {//启动线程
        _threads.emplace_back([i,this](){
            this->_io_contexts[i].run();
        });
    }
}
boost::asio::io_context& IOServerPool::GetIOService()
{   
    return _io_contexts[(_index++)%_pool_size];
}
void IOServerPool::ResetIndex()
{
    if(!_index)
    {//第一个io_context
        _index=_pool_size-1;return;}
    _index--;
}
IOServerPool::~IOServerPool()
{
    for(auto & e: _io_contexts){
        e.stop();}
    for(auto & e : _works){
        e.reset();}
    for(auto& e : _threads){
        if(e.joinable()){
        e.join();}}
}