#include"../include/LogicSystem.h"
#include"../include/HttpConnection.h"
LogicSystem::LogicSystem()
{
    LogicSystem::RegisterGet("/get_test",[](std::shared_ptr<HttpConnection> httpcon){
        boost::beast::ostream(httpcon->_response.body()) << "receive get_test req " << std::endl;
        int i = 0;
        for (auto& elem : httpcon->_get_params) {
            i++;
            boost::beast::ostream(httpcon->_response.body()) << "param" << i << " key is " << elem.first;
            boost::beast::ostream(httpcon->_response.body()) << ", " <<  " value is " << elem.second << std::endl;
        }
    });
    //注册客户端使用post发送获取邮箱的请求的处理函数
    LogicSystem::RegisterPost("/get_varifycode",[](std::shared_ptr<HttpConnection> httpcon){
        auto body_str = boost::beast::buffers_to_string(httpcon->_request.body().data()); //获取请求正文
        std::cout << "receive body is " << body_str << std::endl;
        httpcon->_response.set(boost::beast::http::field::content_type, "text/json");
        nlohmann::json root;                            
        nlohmann::json src_root;                  
        try{           
             src_root=nlohmann::json::parse(body_str);} //反序列化
        catch(const nlohmann::json::parse_error& err)
        { //反序列化失败
            std::cout<<"Fail to parse json "<<err.what()<<std::endl;
            root["error"]=ErrorCodes::Error_Json;
            auto jsonstr=root.dump(4); //序列化
            boost::beast::ostream(httpcon->_response.body()) << jsonstr;
            return;
        }
        if(!src_root.contains("email")) //不含有email字段
        {
            std::cout<<"Fail to get email "<<std::endl;
            root["error"]=ErrorCodes::Error_Json;
            auto jsonstr=root.dump(4); //序列化
            boost::beast::ostream(httpcon->_response.body()) << jsonstr;
            return;
        }
        //客户端json中email字段
        std::cout<<"email is "<<src_root["email"].get<std::string>()<<std::endl;
        //这条语句创建单例VarifyGrpcClient并初始化grpc连接池 
        GateServer::Varify::GetVarifyRsp grpc_rsp=VarifyGrpcClient::GetInstance()->GetVarify(src_root["email"].get<std::string>());
        root["error"]=grpc_rsp.error();  //1.成功则nodejs设置 2.失败则函数内部设置
        root["email"]=src_root["email"].get<std::string>();
        std::string jsonstr = root.dump(4); //序列化
        boost::beast::ostream(httpcon->_response.body()) << jsonstr;
        return;
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
    //找到指定方法并执行处理
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
    //找到指定方法并执行处理
    _post_handlers[path](std::move(httpcon));
    return true;
}