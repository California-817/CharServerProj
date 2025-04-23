#include"../include/HttpConnection.h"
#include"../include/CServer.h"
HttpConnection::HttpConnection(std::shared_ptr<CServer> server,std::shared_ptr<boost::asio::ip::tcp::socket> socket)
:_server(std::move(server)),_socket(std::move(socket)),_timer(_socket->get_executor(),std::chrono::seconds(60)) //初始化定时器事件
{
    boost::uuids::uuid id=boost::uuids::random_generator()();
    _uuid=boost::uuids::to_string(id);
    start(); //开始接收数据
}
std::string& HttpConnection::get_uuid()
{
    return _uuid;
}
void HttpConnection::start()//不断读取数据
{   
    //此时连接建立成功 开始为该连接倒计时 直到在规定事件发送响应之后主动关闭定时器  未在规定时间发送则强制关闭
    HttpConnection::checkDeadline();
    auto self=shared_from_this(); //构造连接的闭包  为了传给回调函数
    //使用http的异步读函数 会读取数据到buffer 并且解析出完整http请求到request中 再调用回调函数  // void(error_code, std::size_t)
    boost::beast::http::async_read(*_socket,_buffer,_requt,[self](boost::system::error_code ec,size_t bytes_transferred){    
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
        }
    });
}

void HttpConnection::handleReq() //处理各种请求 GET　POST
{   
    //设置响应版本 设置短连接: 一次请求与响应就结束tcp连接
    _response.version(_request.version());
    _response.keep_alive(false);
    
}
void HttpConnection::checkDeadline()
{   
    auto self=shared_from_this();
    _timer.async_wait([self](boost::system::error_code ec){   //void (boost::system::error_code)
        if(!ec)
        {
            self->get_socket()->close();
            self->get_server()->clearSession(self->get_uuid());
        }
    });
}   
HttpConnection::~HttpConnection()
{
    if(get_socket()->is_open())
    { //仍是打开的
        get_socket()->close();
    }
}
std::shared_ptr<boost::asio::ip::tcp::socket>& HttpConnection::get_socket()
{
    return _socket;
}
std::shared_ptr<CServer>& HttpConnection::get_server()
{
    return _server;
}