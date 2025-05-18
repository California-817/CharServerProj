#pragma once
#include"Const.h"
#include"RedisMgr.h"
#include"Singleton.h"
//加解分布式锁的单例类
class DistLock : public Singleton<DistLock>
{
public:
    friend class Singleton<DistLock>;
    std::string acquireLock(const std::string& lock_key,int locktimeout,int acquiretime);
    bool releaseLock(const std::string& lock_key,const std::string& uuid);
    ~DistLock()=default;
private:
    DistLock()=default;
    DistLock(const DistLock &)=delete;
    DistLock operator=(const DistLock &)=delete;
};