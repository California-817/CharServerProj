#include "../include/CServer.h"
CServer::CServer(boost::asio::io_context &io_context, uint16_t &port)
    : _io_context(io_context), _acceptor(_io_context,
                                         boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::any(), port))
{
    CServer::start();
}

// 异步接收的回调函数
void CServer::handleAccept(std::shared_ptr<CServer> self, std::shared_ptr<boost::asio::ip::tcp::socket> _socket, boost::system::error_code ec)
{
    try
    {
        if (ec) // 错误
        {
            _socket.reset();
            //再次开始接收连接
            self->start();
            return;
        }
        else
        { // 成功
            auto http_con=std::make_shared<HttpConnection>(std::move(self),std::move(_socket));
            _conns.insert(std::make_pair(http_con->get_uuid(),std::move(http_con)));
            self->start();
            return;
        }
    }
    catch (boost::system::system_error &err)
    {
        std::cout << "accept err " << err.what() << std::endl;
        self->start();
    }
}
void CServer::start() // 开始接收连接
{
    auto self = shared_from_this(); // 创建伪闭包
    // 创建一个用来与一个客户端通信的socket
    std::shared_ptr<boost::asio::ip::tcp::socket> _socket = std::make_shared<boost::asio::ip::tcp::socket>(_io_context);
    _acceptor.async_accept(*_socket, std::bind(CServer::handleAccept, this, self, std::move(_socket), std::placeholders::_1));
}
void CServer::clearSession(std::string& uuid)
{
    if(_conns.find(uuid)!=_conns.end())
    {//找到了
        _conns.erase(uuid); //删除管理的httpconn的指针
    }
}
CServer::~CServer()
{
    _acceptor.close();
    std::cout << "CServer close" << std::endl;
}
