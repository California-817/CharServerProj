#pragma once
#include"Const.h"
class CServer;
class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{
public:
    HttpConnection(std::shared_ptr<CServer> server,std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    std::string& get_uuid();
    std::shared_ptr<CServer>& get_server();
    std::shared_ptr<boost::asio::ip::tcp::socket>& get_socket();
    void start(); //不断读取数据
    void checkDeadline();
    void handleReq(); //处理请求
    ~HttpConnection();
private:
    std::shared_ptr<CServer> _server; //所属的服务器
    std::shared_ptr<boost::asio::ip::tcp::socket> _socket; //底层socket
    std::string _uuid; //连接的唯一id
    boost::beast::flat_buffer _buffer{8192};  //存放读取数据的动态缓冲区 [已消费][还未消费][空闲区域]
    boost::beast::http::request<boost::beast::http::dynamic_body> _request;  //请求结构体
    boost::beast::http::response<boost::beast::http::dynamic_body> _response; //响应结构体
    boost::asio::steady_timer _timer; //定时器事件 到期断开连接
};  