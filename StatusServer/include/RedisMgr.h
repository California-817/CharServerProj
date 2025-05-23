#pragma once
#include"Const.h"
#include"Singleton.h"
#include<sw/redis++/redis++.h>
//直接在pool中添加一个thread线程进行定期检测连接的活跃状态以及存活状态 不对连接本身进行封装时间戳--上层修改工作太大
//新增对redis函数执行的异常的捕获 提高程序的健壮性
class RedisConPool
{
public:
    RedisConPool(const std::string& host,const std::string& port,const std::string& password,
                                                                 size_t size=std::thread::hardware_concurrency());
    std::shared_ptr<sw::redis::Redis> GetRedisCon();
    void CheckConnectionTask();
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
    std::thread _check_thread; //检测连接线程
    std::atomic<int> _failed_count; //无效连接数
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