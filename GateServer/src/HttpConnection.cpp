#include "../include/HttpConnection.h"
// 在头文件中前置声明 在cpp文件中包含头文件 防止循环引用
#include "../include/CServer.h"
#include "../include/LogicSystem.h"
HttpConnection::HttpConnection(std::shared_ptr<CServer> server, std::shared_ptr<boost::asio::ip::tcp::socket> socket)
    : _server(std::move(server)), _socket(std::move(socket)), _timer(_socket->get_executor(), std::chrono::seconds(60)) // 初始化定时器事件
{
    boost::uuids::uuid id = boost::uuids::random_generator()();
    _uuid = boost::uuids::to_string(id);
    //start(); // 开始接收数据  与CServer同理 还没形成智能指针就构造伪闭包
}
std::string &HttpConnection::get_uuid()
{
    return _uuid;
}
void HttpConnection::start() // 不断读取数据
{
    // 此时连接建立成功 开始为该连接倒计时 直到在规定事件发送响应之后主动关闭定时器  未在规定时间发送则强制关闭
    HttpConnection::checkDeadline();
    auto self = shared_from_this(); // 构造连接的闭包  为了传给回调函数
    // 使用http的异步读函数 会读取数据到buffer 并且解析出完整http请求到request中 再调用回调函数  // void(error_code, std::size_t)
    boost::beast::http::async_read(*_socket, _buffer, _request, [self](boost::system::error_code ec, size_t bytes_transferred)
                                   {    
        //读取一个完整http请求后的处理逻辑    lambda表达式定义的回调
        try{
            if(ec)
            {
                std::cout<<"Recv http req err "<<ec.what()<<std::endl;
                self->get_server()->clearSession(self->get_uuid());
                return;
            }
            boost::ignore_unused(bytes_transferred);
            //正确读取并解析了一个http请求  解析处理
            self->handleReq();
        }
        catch(boost::system::system_error & err)
        {
            std::cout<<"Recv http req err "<<err.what()<<std::endl;
        } });
}

void HttpConnection::handleReq() // 处理各种请求 GET　POST
{
    // 设置响应版本 设置短连接: 一次请求与响应就结束tcp连接
    _response.version(_request.version());
    _response.keep_alive(false);
    if (_request.method() == boost::beast::http::verb::get) // get请求
    {
        bool success = LogicSystem::GetInstance()->HandleGet(_request.target(), shared_from_this());
        if (!success)
        { // 逻辑系统中没有存储这个url对应的处理函数
            _response.result(boost::beast::http::status::not_found);
            _response.set(boost::beast::http::field::content_type, "text/plain");
            boost::beast::ostream(_response.body()) << "url not found\r\n"; // 向响应正文写入数据
            WriteResponse();                                                // 发送响应
            return;
        }
        // 逻辑系统中没有存储这个url对应的处理函数
        _response.result(boost::beast::http::status::ok);
        _response.set(boost::beast::http::field::server, "GateServer");
        WriteResponse(); // 发送响应
        return;
    }
}
void HttpConnection::WriteResponse()
{
    auto self = shared_from_this();                    // 实现伪闭包 给一个智能指针给回调函数
    _response.content_length(_response.body().size()); // void(error_code, std::size_t)
    boost::beast::http::async_write(*_socket, _response, [self](boost::system::error_code ec, size_t bytes_transferred)
                                    {  
        try{
            boost::ignore_unused(bytes_transferred);
            self->_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_send,ec); //关闭socket的接收功能 并未真正释放套接字资源s
            self->_timer.cancel();//发送响应了取消定时事件
            self->get_server()->clearSession(self->get_uuid()); //移除管理
        }
        catch(boost::system::system_error& err)
        {
            std::cout<<"Write response err "<<err.what()<<std::endl;
            boost::ignore_unused(bytes_transferred);
            self->_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_send,ec); //关闭socket的接收功能 并未真正释放套接字资源s
            self->_timer.cancel();//发送响应了取消定时事件
            self->get_server()->clearSession(self->get_uuid()); //移除管理
        } });
}
void HttpConnection::checkDeadline()
{
    auto self = shared_from_this();
    _timer.async_wait([self](boost::system::error_code ec) { // void (boost::system::error_code)
        if (!ec)
        {
            self->get_socket()->close();
            self->get_server()->clearSession(self->get_uuid());
        }
    });
}
HttpConnection::~HttpConnection()
{
    if (get_socket()->is_open())
    { // 仍是打开的
        get_socket()->close(); //主动断开可能会造成大量time_wait状态
    }
}
std::shared_ptr<boost::asio::ip::tcp::socket> &HttpConnection::get_socket()
{
    return _socket;
}
std::shared_ptr<CServer> &HttpConnection::get_server()
{
    return _server;
}