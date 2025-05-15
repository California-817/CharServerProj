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
void UserMgr::RmUidSession(int uid)
{
    std::lock_guard<std::mutex> _lock(_session_mtx);
    auto it=_uid_to_sessions.find(uid);
    if(it==_uid_to_sessions.end())
    { //没找到
       return;
    }
    //找到了    这里后面还需要扩展
    _uid_to_sessions.erase(uid);

}
UserMgr::~UserMgr()
{
    std::lock_guard<std::mutex> _lock(_session_mtx);
    _uid_to_sessions.clear();
}