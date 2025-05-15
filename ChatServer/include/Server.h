#pragma once
#include "Session.h"
#include"IOContextPool.h"
#include"MsgNode.h"
#include"UserMgr.h"
#include"RedisMgr.h"
#include"Const.h"
class Session;
class MsgNode;
class IOContextPool;
class Server
{
public:
    Server(std::shared_ptr<boost::asio::io_context> io_context, uint16_t port);
    void ClearSession(std::string& uuid);
    ~Server();
private:
    // 接收完的回调函数
    void HandleAccept(std::shared_ptr<Session> session, const boost::system::error_code &error);
    // 开始接收连接
    void StartAccecp();
    
private:
    // boost异步服务器的成员
    uint16_t _port;
    std::shared_ptr<boost::asio::io_context> _io_context;                // 上下文 相当于libevent的时间分发器
    boost::asio::ip::tcp::acceptor _accecptor;                           // 用于接收客户端的socket连接 相当于listen套接字
    std::unordered_map<std::string, std::shared_ptr<Session>> _sessions; // 管理所有的session 封装连接
    std::mutex _mtx ;  //可能是多线程去删除sessions中的连接
};