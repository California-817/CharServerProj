#include"../include/UserMgr.h"
#include"../include/Session.h"

UserMgr::UserMgr()
{}
void UserMgr::SetUidSession(int uid,std::shared_ptr<Session> session)
{
    std::lock_guard<std::mutex> _lock(_session_mtx);
    _uid_to_sessions.insert(std::make_pair(uid,std::move(session)));
}
std::shared_ptr<Session> UserMgr::GetUidSession(int uid)
{
    std::lock_guard<std::mutex> _lock(_session_mtx);
    auto it=_uid_to_sessions.find(uid);
    if(it!=_uid_to_sessions.end())
    { //找到了
        return it->second;
    }
    return nullptr;
}
void UserMgr::RmUidSession(int uid,const std::string& sessionid)
{
    std::lock_guard<std::mutex> _lock(_session_mtx);
    auto it=_uid_to_sessions.find(uid);
    if(it==_uid_to_sessions.end())
    { //没找到
       return;
    }
    //找到了 判断这个uid对应session是新连接的还是旧连接的
    if(_uid_to_sessions[uid]->GetSessionId()!=sessionid)
    { //不是旧的连接 是新连接
        return;
    }//是旧连接直接删除
    _uid_to_sessions.erase(uid);
}
UserMgr::~UserMgr()
{
    std::lock_guard<std::mutex> _lock(_session_mtx);
    _uid_to_sessions.clear();
}