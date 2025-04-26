#include "../include/CServer.h"
CServer::CServer(boost::asio::io_context &io_context, uint16_t &port)
    : _io_context(io_context), _acceptor(_io_context,
                                         boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::any(), port))
{
    //start()  直接在构造函数调用会报错 因为此时外部还没有生成this的智能指针
    //而start里面会调用shared_from_this 导致报错
}

// 异步接收的回调函数
void CServer::handleAccept(std::shared_ptr<CServer> self, std::shared_ptr<boost::asio::ip::tcp::socket> _socket, boost::system::error_code ec)
{
    try
    {
        if (ec) // 错误
        {
            _socket.reset();
            IOServerPool::GetInstance()->ResetIndex();//index--
            //再次开始接收连接
            self->start();
            return;
        }
        else
        { // 成功
            auto http_con=std::make_shared<HttpConnection>(self,std::move(_socket));
            //在形成http_con这个智能指针后再形成闭包
            http_con->start();
            //这里其实可不用map来管理conn的生命周期 因为在start内部lambda回调函数有绑定一个该对象的智能指针 共享引用计数
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
    //调用这个函数的前提是外部已经有了这个this指针的智能指针 否则会报错 
    auto self = shared_from_this(); // 创建伪闭包
    // 创建一个用来与一个客户端通信的socket 从IOServerPool中挑选一个io_context
    auto& io_context=IOServerPool::GetInstance()->GetIOService(); //这条语句单例生成 线程跑起来了
    std::shared_ptr<boost::asio::ip::tcp::socket> _socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context);
    //这里的_socket不能用std::move 因为这个async_accept正在使用_socket
    _acceptor.async_accept(*_socket, std::bind(&CServer::handleAccept, this, self, _socket, std::placeholders::_1));
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
