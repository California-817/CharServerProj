#pragma once
#include"Const.h"
#include"MysqlPool.hpp"
#include"Singleton.h"
class MysqlMgr: public Singleton<MysqlMgr>
{
public:
    friend class Singleton<MysqlMgr>;
    int RegUser(const std::string& name,const std::string& email,const std::string& password);//注册用户
    bool CheckEmail(const std::string& name,const std::string& email);//看用户和邮箱是否匹配
    bool UpdatePwd(const std::string& name,const std::string& password);//更新密码

    ~MysqlMgr()=default;
private:
    MysqlMgr();
    MysqlMgr(const MysqlMgr&)=delete;
    MysqlMgr& operator=(const MysqlMgr&)=delete;
private:
    std::unique_ptr<ConnectionPool> _pool;
};