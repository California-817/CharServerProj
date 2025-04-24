#include"../include/LogicSystem.h"
#include"../include/HttpConnection.h"
LogicSystem::LogicSystem()
{
    LogicSystem::RegisterGet("/get_test",[](std::shared_ptr<HttpConnection> httpcon){
        boost::beast::ostream(httpcon->_response.body())<<"recv get_test req ";
    });
}
void LogicSystem::RegisterGet(std::string path,HttpHandler handler) 
{
    _get_handlers.insert(std::make_pair(path,handler));
}
bool LogicSystem::HandleGet(std::string path,std::shared_ptr<HttpConnection> httpcon) //处理get请求
{
    if(_get_handlers.find(path)==_get_handlers.end())
    {//没找到指定的方法
        return false;
    }
    _get_handlers[path](std::move(httpcon));
    return true;
}
void LogicSystem::RegisterPost(std::string path,HttpHandler handler)
{
    _post_handlers.insert(std::make_pair(path,handler));
}
bool LogicSystem::HandlePost(std::string path,std::shared_ptr<HttpConnection> httpcon)//处理post请求
{
    if(_post_handlers.find(path)==_post_handlers.end())
    {//没找到指定的方法
        return false;
    }
    _post_handlers[path](std::move(httpcon));
    return true;
}