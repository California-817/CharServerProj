#include"../include/LogicSystem.h"
#include"../include/HttpConnection.h"
LogicSystem::LogicSystem()
{
    //1.客户端get请求的对应处理
    LogicSystem::RegisterGet("/get_test",[](std::shared_ptr<HttpConnection> httpcon){
        boost::beast::ostream(httpcon->_response.body()) << "receive get_test req " << std::endl;
        int i = 0;
        for (auto& elem : httpcon->_get_params) {
            i++;
            boost::beast::ostream(httpcon->_response.body()) << "param" << i << " key is " << elem.first;
            boost::beast::ostream(httpcon->_response.body()) << ", " <<  " value is " << elem.second << std::endl;
        }
    });

    //2.注册客户端使用post发送获取邮箱的请求的处理函数
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
        auto redis_con=RedisMgr::GetInstance()->GetRedisCon();
        //这条语句创建单例VarifyGrpcClient并初始化grpc连接池 
        GateServer::Varify::GetVarifyRsp grpc_rsp=VarifyGrpcClient::GetInstance()->GetVarify(src_root["email"].get<std::string>());
        root["error"]=grpc_rsp.error();  //1.成功则nodejs设置 2.失败则函数内部设置
        root["email"]=src_root["email"].get<std::string>();
        std::string jsonstr = root.dump(4); //序列化
        boost::beast::ostream(httpcon->_response.body()) << jsonstr;
        return;
     });

     //3.注册客户端注册用户的请求对应的处理函数并返回响应
     LogicSystem::RegisterPost("/user_register",[](std::shared_ptr<HttpConnection> httpcon){
        auto body_str=boost::beast::buffers_to_string(httpcon->_request.body().data());
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
        //反序列化成功 获取字段值
        std::string user=src_root["user"].get<std::string>();
        std::string email=src_root["email"].get<std::string>();
        std::string password=src_root["password"].get<std::string>();
        std::string confirm=src_root["confirm"].get<std::string>();
        std::string varifycode=src_root["varifycode"].get<std::string>();
        //先查看redis中验证码是否有效
        std::string prefix="code_";
        prefix+=email;
        auto redis_con=RedisMgr::GetInstance()->GetRedisCon();
        auto ret=redis_con->get(prefix.c_str());
        if(!ret.has_value())
        { //验证码过期
            std::cout << " get varify code expired" << std::endl;
            root["error"] = ErrorCodes::VarifyExpired;
            std::string jsonstr = root.dump(4); //序列化
            boost::beast::ostream(httpcon->_response.body()) << jsonstr;
            return;
        }
        if(ret.value()!=varifycode)
        { //验证码有错误
            std::cout << " varify code error" << std::endl;
            root["error"] = ErrorCodes::VarifyCodeErr;
            std::string jsonstr = root.dump(4);
            boost::beast::ostream(httpcon->_response.body()) << jsonstr;
            return;
        }
        //去mysql数据库注册用户信息
        int uid=MysqlMgr::GetInstance()->RegUser(user,email,password);
        if(uid==-1)
        {
            std::cout << "user exist" << std::endl;
            root["error"] = ErrorCodes::UserExist;
            std::string jsonstr = root.dump(4);
            boost::beast::ostream(httpcon->_response.body()) << jsonstr;
            return;
        }
        if(uid==0)
        {
            std::cout << "email exist" << std::endl;
            root["error"] = ErrorCodes::EmailExist;
            std::string jsonstr = root.dump(4);
            boost::beast::ostream(httpcon->_response.body()) << jsonstr;
            return;
        }

        //返回响应
        root["error"] = ErrorCodes::Success;
        root["uid"] =uid;
        root["email"] = email;
        root ["user"]= user;
        root["password"] = password;
        root["confirm"] = confirm;
        root["varifycode"] = varifycode;
        std::string jsonstr = root.dump(4);
        boost::beast::ostream(httpcon->_response.body()) << jsonstr;
        return ;
     });
     //4.重置密码的处理逻辑
     RegisterPost("/reset_pwd",[](std::shared_ptr<HttpConnection> httpcon){
        auto body_str=boost::beast::buffers_to_string(httpcon->_request.body().data());
        std::cout << "receive body is " << body_str << std::endl;
        httpcon->_response.set(boost::beast::http::field::content_type, "text/json");
        //反序列化body字符串
        nlohmann::json root;
        nlohmann::json src_root;
        try{
            src_root=nlohmann::json::parse(body_str);
        }catch(const nlohmann::json::parse_error& err)
        { //反序列化失败
          std::cout<<"Fail to parse json "<<err.what()<<std::endl;
          root["error"]=ErrorCodes::Error_Json;
          auto jsonstr=root.dump(4); //序列化
          boost::beast::ostream(httpcon->_response.body()) << jsonstr;
          return;
        }
        std::string user=src_root["user"].get<std::string>();
        std::string email=src_root["email"].get<std::string>();
        std::string password=src_root["passwd"].get<std::string>();
        std::string varifycode=src_root["varifycode"].get<std::string>();
        //先去redis查验证码是否过期 是否正确
        std::string prefix="code_";
        prefix+=email;
        auto redis_con=RedisMgr::GetInstance()->GetRedisCon();
        auto ret=redis_con->get(prefix.c_str());
        if(!ret.has_value())
        { //验证码过期
            std::cout << " get varify code expired" << std::endl;
            root["error"] = ErrorCodes::VarifyExpired;
            std::string jsonstr = root.dump(4); //序列化
            boost::beast::ostream(httpcon->_response.body()) << jsonstr;
            return;
        }
        if(ret.value()!=varifycode)
        { //验证码有错误
            std::cout << " varify code error" << std::endl;
            root["error"] = ErrorCodes::VarifyCodeErr;
            std::string jsonstr = root.dump(4);
            boost::beast::ostream(httpcon->_response.body()) << jsonstr;
            return;
        }

        //去mysql查看用户和邮箱是否匹配
        bool check_email=MysqlMgr::GetInstance()->CheckEmail(user,email);
        if(!check_email)
        {  //不匹配
            std::cout<<"user not match email"<<std::endl;
            root["error"] = ErrorCodes::EmailNotMatch;
            std::string jsonstr = root.dump(4);
            boost::beast::ostream(httpcon->_response.body()) << jsonstr;
            return;
        }
        //再更新数据库中的密码
        bool update_pwd=MysqlMgr::GetInstance()->UpdatePwd(user,password);
        if(!update_pwd)
        {
            std::cout<<"update password failed"<<std::endl;
            root["error"] = ErrorCodes::PassWordUpErr;
            std::string jsonstr = root.dump(4);
            boost::beast::ostream(httpcon->_response.body()) << jsonstr;
            return;
        }
         //返回响应
         root["error"] = ErrorCodes::Success;
         root["email"] = email;
         root ["user"]= user;
         root["password"] = password;
         root["varifycode"] = varifycode;
         std::string jsonstr = root.dump(4);
         boost::beast::ostream(httpcon->_response.body()) << jsonstr;
         return ; 
     });
     //5.客户端登录请求的处理逻辑
     //json_obj["user"] = user;
     //json_obj["passwd"] = xorString(pwd);
     //HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/user_login"),
     //                                          json_obj, ReqId::ID_LOGIN_USER,Modules::LOGINMOD);
     LogicSystem::RegisterPost("/user_login",[](std::shared_ptr<HttpConnection> httpcon){
        auto body_str=boost::beast::buffers_to_string(httpcon->_request.body().data());
        std::cout << "receive body is " << body_str << std::endl;
        httpcon->_response.set(boost::beast::http::field::content_type, "text/json");
        //反序列化body字符串
        nlohmann::json root;
        nlohmann::json src_root;
        try{
            src_root=nlohmann::json::parse(body_str);
        }catch(const nlohmann::json::parse_error& err)
        { //反序列化失败
          std::cout<<"Fail to parse json "<<err.what()<<std::endl;
          root["error"]=ErrorCodes::Error_Json;
          auto jsonstr=root.dump(4); //序列化
          boost::beast::ostream(httpcon->_response.body()) << jsonstr;
          return;
        }
        std::string email=src_root["user"].get<std::string>();
        std::string password=src_root["passwd"].get<std::string>();
        //先去mysql查看email和密码是否匹配
        UserInfo userinfo;
        bool check_res=MysqlMgr::GetInstance()->CheckPwd(email,password,userinfo);
        if(!check_res)
        { //不匹配
            std::cout<<"login password is error "<<std::endl;
            root["error"]=ErrorCodes::PassWordErr; //密码错误的错误码
            std::string jsonstr = root.dump(4);
            boost::beast::ostream(httpcon->_response.body()) << jsonstr;
            return;
        }
        //密码与邮箱匹配userinfo填充数据
        //查询StatusServer找到合适的连接
        

        std::cout << "succeed to load userinfo uid is " << userinfo.uid << std::endl;
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