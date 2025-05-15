#include "../include/RedisMgr.h"
RedisConPool::RedisConPool(const std::string &host, const std::string &port, const std::string &password,
                           size_t size)
    : _host(host), _port(port), _password(password), _size(size),_b_stop(false)
{
    std::string str = "tcp://";
    str += host;
    str += ":";
    str += port;
    for (int i = 0; i < _size; i++)
    {
        auto con_ptr = new sw::redis::Redis(str.c_str());// 创建了一个连接
        if(con_ptr==nullptr)
        { //创建连接失败
            std::cout<<"connect to redis fail"<<std::endl;}
        con_ptr->auth(password.c_str());    // 进行身份认证
        _redis_cons.push(con_ptr);    // 入队列
    }
}
std::shared_ptr<sw::redis::Redis> RedisConPool::GetRedisCon()
{
    std::unique_lock<std::mutex> _lock(_mtx);
    while (_redis_cons.empty() && !_b_stop)
    {
        _cv.wait(_lock);
    }
    if (_b_stop)//暂停连接池
    {
        return std::unique_ptr<sw::redis::Redis>(); // 返回空智能指针
    }
    // 有连接
    std::shared_ptr<sw::redis::Redis> con(_redis_cons.front(),[this](sw::redis::Redis* con){ //自定义删除器
        std::lock_guard<std::mutex> _lock(this->_mtx);
        if(this->_b_stop)
        { //连接池关闭了 直接把链接删除
            delete con;
            return;}
        this->_redis_cons.push(con); //将连接归还连接池
        this->_cv.notify_one();
    });
    _redis_cons.pop();
    return con;
}
void RedisConPool::Close()
{
    _b_stop=true;
    _cv.notify_all();
}
RedisConPool::~RedisConPool()
{
    Close();
    std::lock_guard<std::mutex> _lock(_mtx);
    while(!_redis_cons.empty())
    {
        delete _redis_cons.front();
        _redis_cons.pop();
    }
}
RedisMgr::RedisMgr()
{   
    mINI::INIFile file("../conf/config.ini");
    mINI::INIStructure ini;
    file.read(ini);
    std::string host=ini["Redis"]["host"];
    std::string port=ini["Redis"]["port"];
    std::string password=ini["Redis"]["password"];
    size_t size=atoi(ini["RedisPool"]["size"].c_str());
    _pool.reset(new RedisConPool(host,port,password,size));
}
std::shared_ptr<sw::redis::Redis> RedisMgr::GetRedisCon()
{
    return _pool->GetRedisCon();
}


