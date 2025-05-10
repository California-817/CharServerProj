#include"../include/Calldata.h"
#include"../include/StatusServerImpl.h"

Calldata::Calldata(StatusServer::AsyncService* service,ServerCompletionQueue* cq,StatusServerImpl* impl)
    :_service(service),_cq(cq),_status(CREATE),_impl(impl)
    {}

GetChatServerCalldata::GetChatServerCalldata(StatusServer::AsyncService* service, ServerCompletionQueue* cq,StatusServerImpl* impl)
    :Calldata(service,cq,impl),_responder(&_ctx)
    {
        // Invoke the serving logic right away. 立即调用服务逻辑
        Proceed();
    }
void GetChatServerCalldata::Proceed() 
    {
        if(_status==CREATE)
        {
             // Make this instance progress to the PROCESS state.
            _status=PROCESS;
            //将这个calldata实例注册到异步grpc框架用来保存解析到的一个请求并放入完成队列
            //this指针作为这次注册的tag标记,用于从完成队列获取事件时找到该calldata实例
            _service->RequestGetChatServer(&_ctx,&_req,&_responder,_cq,_cq,this);
        }
        else if(_status==PROCESS)
        {   
            //已经获取到了一个请求,现在进行处理请求并生成响应放入grpc框架的完成队列
            //Spawn new instance for next request 重新生成一个Calldata用于下次接收请求
            //要形成一个calldata链,每次一个calldata使用后 需要自动生成一个新的calldata继续接收请求
            new GetChatServerCalldata(_service,_cq,_impl);
            //真正的进行请求处理并生成响应
            std::string prefix("my status server has received : GetChatServer request");
            std::cout<<prefix<<std::endl;
            auto server=getChatServer();
            //这里不对ChatServer的连接数量进行增加 增加在chatserver确认连接建立成功后对redis中对应服务器的连接数进行++
            _rsp.set_host(server._host);
            _rsp.set_port(server._port);
            _rsp.set_error(ErrorCodes::Success);
            _rsp.set_token(generate_unique_string());
            //向redis中缓存用户uid以及token
            insertToken(_req.uid(),_rsp.token());
            //处理完毕
            _status=FINISH;
            //告诉grpc框架处理完毕发送响应 进行一次注册逻辑 当grpc框架处理完毕后通过完成队列返回一个事件
            //此时通过tag获取这个calldata对象进行销毁处理
            _responder.Finish(_rsp,Status::OK,this);
        }
        else
        {
            // Once in the FINISH state, deallocate ourselves (CallData).
            delete this; //删除这个calldata对象
        }
    }

std::string GetChatServerCalldata::generate_unique_string()
{
     // 创建UUID对象
     boost::uuids::uuid uuid = boost::uuids::random_generator()();
     // 将UUID转换为字符串
     std::string unique_string = to_string(uuid);
      return unique_string;
}
ChatServer_struct GetChatServerCalldata::getChatServer()
{
    std::lock_guard<std::mutex> _lock(_impl->_servers_mtx);
    //随便获取第一个服务器
    auto minserver=_impl->_chat_servers.begin()->second;
    auto redis_con=RedisMgr::GetInstance()->GetRedisCon();
    auto ret=redis_con->hget(LOGIN_COUNT,minserver._name.c_str());
    if(!ret.has_value()){  //redis中没有这个chatServer的连接数缓存
        minserver._con_count=INT_MAX; //设置成连接数量无穷大表示无效
    }
    else{ //redis中获取最新的连接数(因为是由其他服务器进行修改,不能直接获取本地的conn数,不是最新的)
        minserver._con_count=atoi(ret.value().c_str());
    }
    for(auto& server:_impl->_chat_servers)
    {
        if(server.second._name==minserver._name){
            continue;}
        //另一个chat服务器
        auto ret=redis_con->hget(LOGIN_COUNT,server.second._name.c_str());
        if(!ret.has_value()){  //redis中没有这个chatServer的连接数缓存
            server.second._con_count=INT_MAX; //设置成连接数量无穷大表示无效
        }
        else{ //redis中获取最新的连接数
            server.second._con_count=atoi(ret.value().c_str());
        }
        //获取到redis中最新的连接数后进行比较
        if(server.second._con_count<minserver._con_count)
        {//更新最小连接数服务器
            minserver=server.second;
        }
    }
    return minserver;
}
void GetChatServerCalldata::insertToken(int uid,std::string token)
{// token插入redis中的uid_tokens哈希结构中
    auto redis_con=RedisMgr::GetInstance()->GetRedisCon();
    std::string uid_str=std::to_string(uid);
    std::string token_key=USERTOKENPREFIX+uid_str; //  "utoken_56"
    redis_con->hset(UID_TOKENS,token_key.c_str(),token.c_str());
}





LoginCalldata::LoginCalldata(StatusServer::AsyncService* service, ServerCompletionQueue* cq,StatusServerImpl* impl)
:Calldata(service,cq,impl),_responder(&_ctx)
{
    // Invoke the serving logic right away. 立即调用服务逻辑
    Proceed();
}
void LoginCalldata::Proceed()
{
    if(_status==CREATE)
    {
        _status=PROCESS;
        _service->RequestLogin(&_ctx,&_req,&_responder,_cq,_cq,this);
    }
    else if(_status==PROCESS)
    {   
        new LoginCalldata(_service,_cq,_impl);
        std::string prefix("my status server has received : Login confirm request");
        std::cout<<prefix<<std::endl;
        //登录验证服务
        std::string uid=std::to_string(_req.uid());
        std::string token=_req.token();
        std::string token_key=USERTOKENPREFIX+uid;
        auto redis_con=RedisMgr::GetInstance()->GetRedisCon();
        auto ret=redis_con->hget(UID_TOKENS,token_key.c_str());
        if(!ret.has_value())
        {//没找到对应的tokens
            _rsp.set_error(ErrorCodes::UidInvalid);
        }
        else
        { //找到对应token
            std::string store_token=ret.value();
            if(store_token!=token)
            { //token无效
                _rsp.set_error(ErrorCodes::TokenInvalid);
            }
            else{
            //token有效
            _rsp.set_error(ErrorCodes::Success);
            _rsp.set_uid(_req.uid());
            _rsp.set_token(token);} 
        }
        _status=FINISH;
        _responder.Finish(_rsp,Status::OK,this);
    }
    else
    {
        // Once in the FINISH state, deallocate ourselves (CallData).
        delete this; //删除这个calldata对象
    }
}