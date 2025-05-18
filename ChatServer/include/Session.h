#pragma once
#include "Server.h"
#include "Const.h"
#include "MsgNode.h"
// 为了让内部由this生成的shared_ptr和类外的已经存在的shared_ptr共享相同引用计数的智能指针（引用计数同步）
class Server;
class RecvNode;
class SendNode;
class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(std::shared_ptr<boost::asio::io_context> io_context, Server *server);
    boost::asio::ip::tcp::socket &GetSocket();
    std::string &GetSessionId();
    void SetUid(int uid);
    int GetUid();
    void NotifyOffline(int uid);
    ~Session();
        void Close();
private:
    // 关闭socket套接字
    void HandleReadHead(const boost::system::error_code &error, size_t bytes_transferred, std::shared_ptr<Session> self_ptr);
    void HandleReadData(const boost::system::error_code &error, size_t bytes_transferred, std::shared_ptr<Session> self_ptr);
    // 因为调用的是async_write会全部写完在调用回调 不需要传入 bytes_transferred参数表示实际写入数量
    void HandleWrite(const boost::system::error_code &error, std::shared_ptr<Session> self_ptr);
    void AsyncReadFull(size_t max_length,std::function<void(const boost::system::error_code&, std::size_t)> handler);
    void AsyncReadLen(std::size_t read_len, std::size_t total_len, 
        std::function<void(const boost::system::error_code&, std::size_t)> handler);
    void DealExceptionSession(); //客户端主动断开连接之后对连接信息的处理

public:
    // 上层调用 不需要额外的Read接口 因为服务器的读时间是从Start函数开始就持续注册的
    void Start();

    void Write(uint16_t msg_id, uint16_t len, const char *data);

private:
    std::shared_ptr<boost::asio::io_context> _io_context; // 连接注册事件所属的上下文
    Server *_server;                                      // 连接所属的server
    boost::asio::ip::tcp::socket _socket;                 // 连接的socket结构
    std::string _session_id;                             // 唯一标识这个session
    int _uid; //这个session对应的uid
    bool _b_close;
    // 接收数据的两个结构 --简单方法进行粘包处理 三个数据结构是原始复杂处理
     // 1.直接从socket接收缓冲区获取
     char _data[MAX_LENGTH];
    // 1.获取头部字段
    std::shared_ptr<MsgNode> _recv_head_node;
    // 3.获取正文数据
    std::shared_ptr<RecvNode> _recv_data_node;
    // 发送的一个队列
    std::queue<std::shared_ptr<SendNode>> _send_que;
    // 发送的锁
    std::mutex _mtx;
};