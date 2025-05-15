#pragma once
#include"Const.h"
#include"Singleton.h"
#include<sw/redis++/redis++.h>
class RedisConPool
{
public:
    RedisConPool(const std::string& host,const std::string& port,const std::string& password,
                                                                 size_t size=std::thread::hardware_concurrency());
    std::shared_ptr<sw::redis::Redis> GetRedisCon();
    void Close();
    ~RedisConPool();
private:
    std::queue<sw::redis::Redis*> _redis_cons;
    std::mutex _mtx;
    std::condition_variable _cv;
    size_t _size;
    std::atomic<bool> _b_stop;
    std::string _host;
    std::string _port;
    std::string _password;
};
class RedisMgr : public Singleton<RedisMgr>
{
public:
    friend class Singleton<RedisMgr>;
    std::shared_ptr<sw::redis::Redis> GetRedisCon();
    ~RedisMgr()=default;
private:
    RedisMgr();
    RedisMgr(const RedisMgr&)=delete;
    RedisMgr& operator=(const RedisMgr&)=delete;
    std::unique_ptr<RedisConPool> _pool;
};