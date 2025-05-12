#pragma once
#include"Const.h"
#include"MysqlPool.hpp"
#include"Singleton.h"
class MysqlMgr: public Singleton<MysqlMgr>
{
public:
    friend class Singleton<MysqlMgr>;
    bool GetUserInfo(int uid,UserInfo& userinfo); //根据uid查找用户信息
    bool GetUserInfo(std::string name,UserInfo& userinfo); //函数重载 根据name查找用户信息
    ~MysqlMgr()=default;
private:
    MysqlMgr();
    MysqlMgr(const MysqlMgr&)=delete;
    MysqlMgr& operator=(const MysqlMgr&)=delete;
private:
    std::unique_ptr<ConnectionPool> _pool;
};