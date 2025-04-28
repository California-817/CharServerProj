#pragma once
#include"Const.h"
#include"Singleton.h"
#include"VarifyGrpcClient.h"
#include"RedisMgr.h"
//单例逻辑处理类
class HttpConnection;
using HttpHandler=std::function<void(std::shared_ptr<HttpConnection>)>;
class LogicSystem: public Singleton<LogicSystem>
{
public:
    friend class Singleton<LogicSystem>;
    bool HandleGet(std::string path,std::shared_ptr<HttpConnection> httpcon);
    bool HandlePost(std::string path,std::shared_ptr<HttpConnection> httpcon);
    ~LogicSystem()=default;
private:
    LogicSystem();
    void RegisterGet(std::string path,HttpHandler handler);
    void RegisterPost(std::string path,HttpHandler handler);
    LogicSystem(const LogicSystem&)=delete;
    const LogicSystem& operator=(const LogicSystem&)=delete;
private:
    std::unordered_map<std::string,HttpHandler> _get_handlers; //get请求的url与处理函数的映射集合
    std::unordered_map<std::string,HttpHandler> _post_handlers; //post请求的url与处理函数的映射集合
};