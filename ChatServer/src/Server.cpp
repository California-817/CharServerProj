#include "../include/Server.h"
Server::Server(std::shared_ptr<boost::asio::io_context> io_context, uint16_t port)
    : _port(port), _io_context(io_context), _accecptor(*_io_context, boost::asio::ip::tcp::v4())
{
    _accecptor.bind(boost::asio::ip::tcp::endpoint(
        boost::asio::ip::address_v4::any(), _port)); // 绑定端点
    _accecptor.listen();                             // 设置监听状态
    boost::asio::socket_base::reuse_address option(true); //设置端口复用
    boost::system::error_code ercode;
    _accecptor.set_option(option,ercode);
    if(ercode)
    {
        std::cout<<"accecptor设置端口复用失败"<<std::endl;
    }
    StartAccecp();                                   //              开始接收连接
}
void Server::ClearSession(std::string &session_id)
{ //要进行两个存储结构中session的删除 删除redis中uid与ip映射关系 更改redis中服务器连接数
    auto redis_con=RedisMgr::GetInstance()->GetRedisCon();
    mINI::INIFile file("../conf/config.ini");
    mINI::INIStructure ini;
    file.read(ini);
    auto ret_redis=redis_con->hget(LOGIN_COUNT,ini["SelfServer"]["name"].c_str());
    //更新连接数-1并写入redis  ---未来会用到分布式锁
    redis_con->hset(LOGIN_COUNT,ini["SelfServer"]["name"].c_str(),std::to_string((atoi(ret_redis.value().c_str())-1)));
    //删除redis中缓存的uid与ip的映射关系
    std::string ip_key=USERIPPREFIX; // uip_1
    ip_key+=std::to_string(_sessions[session_id]->GetUid());
    redis_con->hdel(UID_IPS,ip_key.c_str());
    
    if(_sessions.find(session_id)!=_sessions.end())
    { //删除uid与session映射管理中的session
        UserMgr::GetInstance()->RmUidSession(_sessions[session_id]->GetUid()); //这个函数内部会加一把锁
    }
    //可能是多个线程调用来删除 这里的锁加在下面而不是上面 是为了防止嵌套加多把锁造成死锁情况产生
    std::lock_guard<std::mutex> _lock(_mtx);
    _sessions.erase(session_id);
}
// 接收完的回调函数
void Server::HandleAccept(std::shared_ptr<Session> session, const boost::system::error_code &error)
{
    if (!error.value()) // 表示asio的接收操作成功
    {
        session->Start();
        // 插入unordered_map中由server管理 当所有智能指针的引用计数0的时候释放session
        _sessions.insert(std::make_pair(session->GetSessionId(), session));
    }
    else
    {
        // 这次asio接收连接失败
        std::cout << "accept error" << error.what() << std::endl;
    }
    // 继续进行下次异步接收
    StartAccecp();
}
// 开始接收连接
void Server::StartAccecp()
{
    // 异步接收直接返回
    // 创建一个智能指针管理session资源
    //将socket的上下文结构设置成iocontextpool中的iocontext
    std::shared_ptr<boost::asio::io_context> sock_ioc=IOContextPool::GetInstance()->GetIOContext();
    auto session = std::make_shared<Session>(std::move(sock_ioc), this);
    // 本质是注册一个事件
    _accecptor.async_accept(session->GetSocket(), std::bind(&Server::HandleAccept,
                                                            this, session, std::placeholders::_1));
}
Server::~Server()
{
    _accecptor.close();
}
