#pragma once
#include"Const.h"
#include"Singleton.h"
class Session;
class UserMgr: public Singleton<UserMgr>
{
public:
    friend class Singleton<UserMgr>;
    void SetUidSession(int uid,std::shared_ptr<Session> session);
    std::shared_ptr<Session> GetUidSession(int uid);
    void RmUidSession(int uid);
    ~UserMgr();
private:
    UserMgr();
    UserMgr(const UserMgr&)=delete;
    UserMgr& operator=(const UserMgr&)=delete;
private:
    std::mutex _session_mtx;
    std::unordered_map<int,std::shared_ptr<Session>> _uid_to_sessions;
};