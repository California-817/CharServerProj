#pragma once
#include"Const.h"
class CServer;
class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{
public:
    friend class LogicSystem;
    HttpConnection(std::shared_ptr<CServer> server,std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    std::string& get_uuid();
    std::shared_ptr<CServer>& get_server();
    std::shared_ptr<boost::asio::ip::tcp::socket>& get_socket();
    void start(); //不断读取数据
    void checkDeadline();
    void handleReq(); //处理请求
    void WriteResponse();
    unsigned char ToHex(unsigned char x);
    unsigned char FromHex(unsigned char x); //转10进制
    std::string UrlEncode(const std::string& str); //url的编码工作
    std::string UrlDecode(const std::string& str); //url解码工作
    void PreParseGetParam();//进行get请求的解析分出 url 后面的参数 key value
    ~HttpConnection();
private:
    std::shared_ptr<CServer> _server; //所属的服务器
    std::shared_ptr<boost::asio::ip::tcp::socket> _socket; //底层socket
    std::string _uuid; //连接的唯一id
    boost::beast::flat_buffer _buffer{8192};  //存放读取数据的动态缓冲区 [已消费][还未消费][空闲区域]
    boost::beast::http::request<boost::beast::http::dynamic_body> _request;  //请求结构体
    boost::beast::http::response<boost::beast::http::dynamic_body> _response; //响应结构体
    boost::asio::steady_timer _timer; //定时器事件 到期断开连接
    std::string _get_url; //获取get请求的单url
    std::unordered_map<std::string, std::string> _get_params; //获取get请求的参数key value
};  