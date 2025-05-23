#include "../include/RedisMgr.h"
RedisConPool::RedisConPool(const std::string &host, const std::string &port, const std::string &password,
                           size_t size)
    : _host(host), _port(port), _password(password), _size(size), _b_stop(false),_failed_count(0)
{
    std::string str = "tcp://";
    str += host;
    str += ":";
    str += port;
    for (int i = 0; i < _size; i++)
    {
        try
        {
            auto con_ptr = new sw::redis::Redis(str.c_str()); // 创建了一个连接
            con_ptr->auth(password.c_str());                  // 进行身份认证
            _redis_cons.push(con_ptr);                        // 入队列
        }
        catch (std::exception &e)
        {
            std::cout << "connect to redis fail" << e.what() << std::endl;
            _failed_count++;
        }
    }
    // 开启线程进行定时检测连接状态
    _check_thread = std::thread([this](){
        //每隔60s进行一次检测
        int count=0;
        while(!_b_stop){
            if(count>=REDIS_CHECK_TIME){
                this->CheckConnectionTask();
                count=0;
            }
            count++;
            std::this_thread::sleep_for(std::chrono::seconds(1)); //防止长时间等待
        }});
    _check_thread.detach(); //分离线程
}
void RedisConPool::CheckConnectionTask()
{
    int cur_count = 0; // 获取数量的参考值
    {
        std::lock_guard<std::mutex> _lock(_mtx);
        cur_count = _redis_cons.size();
    }
    while (cur_count--)
    {
        sw::redis::Redis *redis_Con;
        {
            std::lock_guard<std::mutex> _lock(_mtx);
            if (_redis_cons.empty() || _b_stop)
                break;
            // 有连接
            redis_Con = _redis_cons.front();
            _redis_cons.pop();
        }
        // 获取了连接 并且连接会不会自动放回
        // 1.对连接进行心跳包发送PING
        bool IsHealthy = true;
        try
        {
            auto ret = redis_Con->ping();
            if (ret != std::string("PONG"))
            {
                // 服务器未正确回PONG 认为连接出错
                _failed_count++;
                IsHealthy = false;
                delete redis_Con; // 析构函数会自动关闭连接
            }
        }
        catch (std::exception &e)
        { // 发送失败说明连接失效 关闭连接
            _failed_count++;
            std::cout << "connect to redis error" << e.what() << std::endl;
            IsHealthy = false;
            delete redis_Con; // 析构函数会自动关闭连接
        }
        // 2.否则放回连接池
        if (IsHealthy)
        {
            // 连接正常
            std::lock_guard<std::mutex> _lock(_mtx);
            _redis_cons.push(redis_Con);
            _cv.notify_one();
        }
    }
    // 3.根据统计的失效连接数重连服务器
    while (_failed_count)
    {
        try
        {
            std::string str = "tcp://";
            str += _host;
            str += ":";
            str += _port;
            auto con_ptr = new sw::redis::Redis(str.c_str()); // 创建了一个连接
            con_ptr->auth(_password.c_str());                 // 进行身份认证
            _redis_cons.push(con_ptr);                        // 入队列
            _failed_count--; //重连成功 无效连接数--
        }
        catch (std::exception &e)
        { //重连失败 这次不再重连 60s后再次尝试
            std::cout << "reconnect to redis fail" << e.what() << std::endl;
            break;
        }
    }
}
std::shared_ptr<sw::redis::Redis> RedisConPool::GetRedisCon()
{
    std::unique_lock<std::mutex> _lock(_mtx);
    while (_redis_cons.empty() && !_b_stop)
    {
        _cv.wait(_lock);
    }
    if (_b_stop) // 暂停连接池
    {
        return std::unique_ptr<sw::redis::Redis>(); // 返回空智能指针
    }
    // 有连接
    std::shared_ptr<sw::redis::Redis> con(_redis_cons.front(), [this](sw::redis::Redis *con) { // 自定义删除器
        std::lock_guard<std::mutex> _lock(this->_mtx);
        if (this->_b_stop)
        { // 连接池关闭了 直接把链接删除
            delete con;
            return;
        }
        this->_redis_cons.push(con); // 将连接归还连接池
        this->_cv.notify_one();
    });
    _redis_cons.pop();
    return con;
}
void RedisConPool::Close()
{
    _b_stop = true;
    _cv.notify_all();
}
RedisConPool::~RedisConPool()
{
    Close();
    std::lock_guard<std::mutex> _lock(_mtx);
    while (!_redis_cons.empty())
    {
        delete _redis_cons.front(); // 防止内存泄漏 析构函数自动关闭连接
        _redis_cons.pop();
    }
}
RedisMgr::RedisMgr()
{
    mINI::INIFile file("../conf/config.ini");
    mINI::INIStructure ini;
    file.read(ini);
    std::string host = ini["Redis"]["host"];
    std::string port = ini["Redis"]["port"];
    std::string password = ini["Redis"]["password"];
    size_t size = atoi(ini["RedisPool"]["size"].c_str());
    _pool.reset(new RedisConPool(host, port, password, size));
}
std::shared_ptr<sw::redis::Redis> RedisMgr::GetRedisCon()
{
    return _pool->GetRedisCon();
}
