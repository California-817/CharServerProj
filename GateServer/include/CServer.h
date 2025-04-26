#pragma once
#include"Const.h"
#include"HttpConnection.h"
#include"IOServicePool.h"
class CServer:public std::enable_shared_from_this<CServer>
{
public:
    CServer(boost::asio::io_context& io_context,uint16_t& port);
    void start();
    void clearSession(std::string& uuid);
    ~CServer();
private:
    void handleAccept(std::shared_ptr<CServer> self,std::shared_ptr<boost::asio::ip::tcp::socket> _socket,boost::system::error_code ec);
private:
    boost::asio::io_context& _io_context; //事件循环
    boost::asio::ip::tcp::acceptor _acceptor; //接收连接
    std::unordered_map<std::string,std::shared_ptr<HttpConnection>> _conns; //管理所有tcp连接
};

