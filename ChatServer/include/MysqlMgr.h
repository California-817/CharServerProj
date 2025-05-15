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
    bool AddFriendApply(int from_uid,int to_uid); //向friend_apply表中添加一条好友申请记录
    bool GetFriendApply(int uid,std::vector<std::shared_ptr<ApplyInfo>>& applylist,int begin,int end); //获取这个user的所有申请人列表
    bool GetFriendList(int uid,std::vector<std::shared_ptr<UserInfo>>& friendlist); //获取这个user的所有好友列表
    ~MysqlMgr()=default;
private:
    MysqlMgr();
    MysqlMgr(const MysqlMgr&)=delete;
    MysqlMgr& operator=(const MysqlMgr&)=delete;
private:
    std::unique_ptr<ConnectionPool> _pool;
};