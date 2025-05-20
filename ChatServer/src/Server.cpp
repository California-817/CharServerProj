#include "../include/Server.h"
Server::Server(std::shared_ptr<boost::asio::io_context> io_context, uint16_t port)
    : _port(port), _io_context(io_context), _accecptor(*_io_context, boost::asio::ip::tcp::v4()), _timer(*io_context)
{
    _accecptor.bind(boost::asio::ip::tcp::endpoint(
        boost::asio::ip::address_v4::any(), _port));      // 绑定端点
    _accecptor.listen();                                  // 设置监听状态
    boost::asio::socket_base::reuse_address option(true); // 设置端口复用
    boost::system::error_code ercode;
    _accecptor.set_option(option, ercode);
    if (ercode)
    {
        std::cout << "accecptor设置端口复用失败" << std::endl;
    }
    // 启动定时检测---检测超时在主线程的io_context的事件循环中进行
    start_timeout_check();
    StartAccecp(); // 开始接收连接
}
void Server::ClearSession(std::string &session_id)
{ // 只进行两个存储结构中session的删除 不进行redis中一些数据的修改 ---涉及到分布式锁来保证安全性
    // 可能是多个io线程调用来删除
    std::lock_guard<std::mutex> _lock(_mtx); //_sessions.find(session_id)防止查找的时候进行修改
    if (_sessions.find(session_id) != _sessions.end())
    {                                                                                      // 删除uid与session映射管理中的session
        UserMgr::GetInstance()->RmUidSession(_sessions[session_id]->GetUid(), session_id); // 这个函数内部会加一把锁
    }
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
    // 将socket的上下文结构设置成iocontextpool中的iocontext
    std::shared_ptr<boost::asio::io_context> sock_ioc = IOContextPool::GetInstance()->GetIOContext();
    auto session = std::make_shared<Session>(std::move(sock_ioc), this);
    // 本质是注册一个事件
    _accecptor.async_accept(session->GetSocket(), std::bind(&Server::HandleAccept,
                                                            this, session, std::placeholders::_1));
}
Server::~Server()
{
    _accecptor.close();
}

bool Server::CheckValid(const std::string &sessionid)
{
    auto it = _sessions.find(sessionid);
    if (it == _sessions.end())
    {
        return false;
    }
    return true;
}

// 开启一次超时检测
void Server::start_timeout_check()
{
    _timer.expires_from_now(boost::posix_time::seconds(CHECK_TIMEOUT)); 
    _timer.async_wait(std::bind(&Server::on_timer, this, std::placeholders::_1));
}
// 超时回调函数
void Server::on_timer(const boost::system::error_code &error)
{ // 处理超时连接
    // 这里先去遍历session将所有超时连接拿出 解锁后再进行连接销毁处理 防止同时加多把锁因加锁顺序不一致导致的死锁问题（保证加锁顺序一致性）
    // std::scoped_lock可用来同时获取多把锁并且避免死锁 原理是try_lock尝试加锁 不成功则释放之前获取的锁 之后再重新尝试获取所有锁
    std::vector<std::shared_ptr<Session>> expired_sessions; 
    int now_session_num = 0; // 统计未超时的session数量
    {
        std::lock_guard<std::mutex> _lock(_mtx);  //这把锁的粒度比较大 考虑进行优化
        auto iter = _sessions.begin();
        for (iter; iter != _sessions.end(); iter++)
        {
            if (iter->second->IsHeartbeatExpired())
            { // 超时
                iter->second->Close();
                expired_sessions.push_back(iter->second);
            }
            else{
                now_session_num++;
            }
        }
    }
    // 进行连接数的重新设置 ---加一把系统级别分布式锁
    {
        mINI::INIFile file("../conf/config.ini");
        mINI::INIStructure ini;
        file.read(ini);
        auto redis_con=RedisMgr::GetInstance()->GetRedisCon();
        std::string identifer_count = DistLock::GetInstance()->acquireLock(LOCK_COUNT, LOCKTIMEOUT, ACQUIRETIME);
        Defer defer_countlock([identifer_count]() { // 出{}自动解锁
            DistLock::GetInstance()->releaseLock(LOCK_COUNT, identifer_count);
        });
        //更新连接数并写入redis
        redis_con->hset(LOGIN_COUNT, ini["SelfServer"]["name"].c_str(), std::to_string(now_session_num));
    }
    //进行session的销毁--先加分布式用户级锁 再加session的线程锁
    for(auto& session:expired_sessions)
    {
        session->DealExceptionSession();
    }
    // 重新开启一次超时检测
    start_timeout_check();
}
