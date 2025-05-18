#include "../include/ChatGrpcServerImpl.h"
#include"../include/Server.h"
ChatServerImpl::ChatServerImpl(Server* p_server)
:_p_server(p_server)
{}
void ChatServerImpl::Run(uint16_t port)
{
    std::string server_address = "0.0.0.0:";
    server_address += std::to_string(port);
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&_service);
    _cq = builder.AddCompletionQueue();
    _server = builder.BuildAndStart();
    std::cout << "Server listening on " << server_address << std::endl;
    for (int i = 0; i < std::thread::hardware_concurrency(); i++)
    { //多个子线程等待grpc请求并进行逻辑处理
        _threads.emplace_back(&ChatServerImpl::HandleRpcs, this);
    }
    // boost::asio::io_context io_context;
    // boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    // signals.async_wait([this](const boost::system::error_code &error, int signal_number)
    //                    {
    //         if(!error){
    //         std::cout << "Shutdown signal received. Stopping server..." << std::endl;
    //         this->_server->Shutdown(); // Shutdown the server
    //         this->_cq->Shutdown();   // Shutdown the completion queue
    //         } });
    // 在单独的子线程中运行io_context
    // std::thread([&io_context]()
    //             { io_context.run(); })
    //     .detach(); // 分离子线程
    //主线程执行到这直接退出
}
// This can be run in multiple threads if needed.
void ChatServerImpl::HandleRpcs()
{
    // 如果有多种请求 需要每种创建一个calldata进行注册到grpc框架接受请求返回事件到cq
    new NotifyAddFriendCalldata(&_service, _cq.get(), this); //添加好友的请求放入完成队列
    new NotifyAuthFriendCalldata(&_service, _cq.get(), this); //认证好友的请求放入完成队列
    new NotifyTextChatMsgCalldata(&_service, _cq.get(), this); //文本消息通信的请求放入完成队列
    new NotifyKickUserCalldata(&_service, _cq.get(), this);//踢人通知请求放入完成对列
    void *tag; // uniquely identifies a request.
    bool ok;
    while (true)
    {
        // Block waiting to read the next event from the completion queue. The
        // event is uniquely identified by its tag, which in this case is the
        // memory address of a CallData instance.
        // Next方法：这是一个阻塞方法，用于从队列中获取完成的事件。它可以在多个线程中并发调用，而不会导致数据竞争
        if (!_cq->Next(&tag, &ok))
        {
            break; // Exit loop if Next() returns false (e.g., on shutdown)
        }
        if (!ok)
        {
            continue;
        }
        static_cast<Calldata *>(tag)->Proceed(); // 多态进行处理
    }
}
ChatServerImpl::~ChatServerImpl()
{
    std::cout << "Shutdown signal received. Stopping grpc server..." << std::endl;
    for (auto &th : _threads)
    {
        if (th.joinable())
        { //主线程等待子线程退出
            th.join();
        } 
    }
}

bool ChatServerImpl::GetUserInfo(int uid,UserInfo& userinfo)
{
    //1.先去redis中获取用户信息
    auto redis_con=RedisMgr::GetInstance()->GetRedisCon();
    if(redis_con.get()==nullptr)
    {
        return false;}
    std::string info_key=USER_BASE_INFO;
    info_key+=std::to_string(uid);   // ubaseinfo_1
    // std::unordered_map<std::string,std::string> redis_ret;
    // auto redis_ret=redis_con->hgetall<std::unordered_map<std::string, std::string>>(info_key);
    // 用于存储结果的容器
    std::unordered_map<std::string, std::string> redis_ret;
    std::vector<std::string> fields{"uid","name","email","nick","pwd","desc","icon","sex"};
    // 对每个字段调用 hget
    for (const auto& field : fields) {
        auto value = redis_con->hget(info_key, field);
        if (value.has_value()) {
            redis_ret[field] = value.value();
        }
    }
    if(!redis_ret.empty())
    { //redis中有该用户信息的缓存
        userinfo._uid=atoi(redis_ret["uid"].c_str());
        userinfo._name=redis_ret["name"];
        userinfo._email=redis_ret["email"];
        userinfo._nick=redis_ret["nick"];
        userinfo._pwd=redis_ret["pwd"];
        userinfo._desc=redis_ret["desc"];
        userinfo._icon=redis_ret["icon"];
        userinfo._sex=atoi(redis_ret["sex"].c_str());
        return true;
    }
    //redis中没有数据缓存 到MySQL获取 并且写入redis
    bool ret=MysqlMgr::GetInstance()->GetUserInfo(uid,userinfo);
    if(ret)
    { //成功获取并写入了userinfo  写入redis  
        redis_con->hmset(info_key,{std::make_pair("uid", std::to_string(userinfo._uid).c_str()),
            std::make_pair("name", userinfo._name.c_str()),
            std::make_pair("nick", userinfo._nick.c_str()),
            std::make_pair("pwd", userinfo._pwd.c_str()),
            std::make_pair("desc", userinfo._desc.c_str()),
            std::make_pair("icon", userinfo._icon.c_str()),
            std::make_pair("sex", std::to_string(userinfo._sex).c_str()),
            std::make_pair("email", userinfo._email.c_str())});
        return true;
    }
    return false;   
}
