#include "../include/Session.h"
Session::Session(std::shared_ptr<boost::asio::io_context> io_context, Server *server)
    : _server(server), _io_context(io_context), _socket(*_io_context) /*不打开 让os打开*/, _b_close(false)
{
    // 创建一个uuid唯一标识session便于管理
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    _uuid = boost::uuids::to_string(a_uuid);
    _recv_head_node = std::make_shared<MsgNode>(HEAD_DATA_LEN);
}
boost::asio::ip::tcp::socket &Session::GetSocket()
{
    return _socket;
}
std::string &Session::GetUuid()
{
    return _uuid;
}
Session::~Session()
{
    // 不用干什么 因为都用智能指针管理着资源
    std::cout<<_uuid<<"client close"<<std::endl;
    // 不需要关闭socket因为要销毁之前已经由上层调用了Close函数进行关闭
}

// 关闭socket套接字
void Session::Close()
{
    _socket.close(); // 1.从io_context中移除 2.关闭文件描述符
    _b_close = true;
}
void Session::HandleReadData(const boost::system::error_code &error, size_t bytes_transferred, std::shared_ptr<Session> self_ptr)
{
    try
    {
        if (!error.value())
        {
            // 读取包体长度成功 形成逻辑包 扔到逻辑队列
            memcpy(_recv_data_node->_data,_data,_recv_data_node->_total_len);
            auto logic_msg = std::make_shared<LogicNode>(self_ptr, _recv_data_node);
            LogicSystem::GetInstance()->PostMsgToQue(std::move(logic_msg));
            // 继续进行读取头部数据
            _recv_head_node->Clear();
            Session::AsyncReadFull(HEAD_TOTAL_LEN,
                                    std::bind(&Session::HandleReadHead, this, std::placeholders::_1, std::placeholders::_2,
                                              shared_from_this()));
        }
        else
        {
            // 读取错误 可能对方断开连接 需要销毁连接 减少智能指针引用计数
            std::cout << "Read cilent error uuid: " << _uuid << std::endl;
            Close();                      // 关闭socket
            _server->ClearSession(_uuid); // 销毁socket结构
        }
    }
    catch (boost::system::system_error &exp)
    {
        std::cout << "file:" << __FILE__ << " " << "line" << __LINE__ << std::endl;
        std::cout << exp.code().value() << std::endl;
        std::cout << exp.what() << std::endl;
    }
}
void Session::HandleReadHead(const boost::system::error_code &error, size_t bytes_transferred, std::shared_ptr<Session> self_ptr)
{
    try
    {

        if (!error.value())
        {
            // 正常读取到了头部字段
            memcpy(_recv_head_node->_data,_data,HEAD_TOTAL_LEN);
            // 头部字段进行解析
            uint16_t msg_id;
            uint16_t msg_len;
            memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
            memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
            // 转本地字节序
            msg_id = ntohs(msg_id);
            msg_len = ntohs(msg_len);
            if (msg_len > MAX_LENGTH) // 包体长度大于最大长度 关闭连接
            {
                Close();                      // 关闭socket
                _server->ClearSession(_uuid); // 销毁socket结构
                return;
            }
            // 长度符合条件
            _recv_data_node.reset();
            _recv_data_node = std::make_shared<RecvNode>(msg_id, msg_len);
            Session::AsyncReadFull( msg_len,
                                    std::bind(&Session::HandleReadData, this, std::placeholders::_1, std::placeholders::_2,
                                              shared_from_this()));
        }
        else
        {
            // 读取错误 可能对方断开连接 需要销毁连接 减少智能指针引用计数
            std::cout << "Read cilent error uuid: " << _uuid << std::endl;
            Close();                      // 关闭socket
            _server->ClearSession(_uuid); // 销毁socket结构
        }
    }
    catch (boost::system::system_error &exp)
    {
        std::cout << "file:" << __FILE__ << " " << "line" << __LINE__ << std::endl;
        std::cout << exp.code().value() << std::endl;
        std::cout << exp.what() << std::endl;
    }
}
// 因为调用的是async_write会全部写完在调用回调 不需要传入 bytes_transferred参数表示实际写入数量
void Session::HandleWrite(const boost::system::error_code &error, std::shared_ptr<Session> self_ptr)
{
    // self_ptr是用来保证每次调用这个函数都最少有一个引用计数 防止智能指针指向资源被销毁
    try
    {
        if (!error.value())
        {
            std::lock_guard<std::mutex> _lock(_mtx);
            _send_que.pop();
            if (!_send_que.empty())
            {
                boost::asio::async_write(_socket, boost::asio::buffer(_send_que.front()->_data, _send_que.front()->_total_len),
                                         std::bind(&Session::HandleWrite, this, std::placeholders::_1, self_ptr));
                return;
            }
            else
                return;
        }
        else
        {
            // 写入失败
            std::cout << "Send cilent error uuid: " << _uuid << std::endl;
            Close();                      // 关闭socket
            _server->ClearSession(_uuid); // 销毁socket结构
        }
    }
    catch (boost::system::system_error &exp)
    {
        std::cout << "file:" << __FILE__ << " " << "line" << __LINE__ << std::endl;
        std::cout << exp.code().value() << std::endl;
        std::cout << exp.what() << std::endl;
    }
}

// 上层调用 不需要额外的Read接口 因为服务器的读时间是从Start函数开始就持续注册的
void Session::Start()
{
    // session开始工作
    _recv_head_node->Clear();
    // 异步读事件  调用enable_shared_from_this 的成员函数 shared_from_this 保证生产的智能指针和unordered_map的引用计数同步
    // 这个函数只会在读取指定长度的数据之后才会调用回调函数
    Session::AsyncReadFull(HEAD_TOTAL_LEN,
                            std::bind(&Session::HandleReadHead, this, std::placeholders::_1, std::placeholders::_2,
                                      shared_from_this()));
}
void Session::Write(uint16_t msg_id, uint16_t len, const char *data)
{
    static int count=0;
    std::cout<<count++<<": "<<data<<std::endl;
    {
        std::lock_guard<std::mutex> _lock(_mtx);
        // 当发送队列满了的时候 不再发送这个包
        if (_send_que.size() == MAX_SENDLEN || _send_que.size() > MAX_SENDLEN)
        {
            std::cout << "session: " << _uuid << " send queue fulled" << std::endl;
            return;
        }
        if (!_send_que.empty()) // 发送队列仍有数据
        {
            _send_que.emplace(std::make_shared<SendNode>(msg_id, len, data));
            return;
        }
        // 队列为空
        _send_que.emplace(std::make_shared<SendNode>(msg_id, len, data));
    }
    boost::asio::async_write(_socket, boost::asio::buffer(_send_que.front()->_data, _send_que.front()->_total_len),
                             std::bind(&Session::HandleWrite, this, std::placeholders::_1, shared_from_this()));
}

void Session::AsyncReadFull(size_t max_length,std::function<void(const boost::system::error_code&, std::size_t)> handler)
{
    memset(_data,0,sizeof _data); //清空data
    Session::AsyncReadLen(0,max_length,handler);
}
void Session::AsyncReadLen(std::size_t read_len, std::size_t total_len, 
    std::function<void(const boost::system::error_code&, std::size_t)> handler)
{
    auto self=shared_from_this();
    _socket.async_read_some(boost::asio::buffer(_data+read_len,total_len-read_len),
        [read_len,total_len,handler,self](const boost::system::error_code& ec, std::size_t  bytesTransfered){
            if(ec)
            { //读取出错
                handler(ec,read_len+bytesTransfered);
                return;
            }
            if(read_len+bytesTransfered>=total_len)
            { //读取完整
                handler(ec,read_len+bytesTransfered);
                return;
            }
            //还没有读完整
          self->AsyncReadLen(read_len+bytesTransfered,total_len,handler);
    });
}