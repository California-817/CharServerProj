#include "../include/Session.h"
Session::Session(std::shared_ptr<boost::asio::io_context> io_context, Server *server)
    : _server(server), _io_context(io_context), _socket(*_io_context) /*不打开 让os打开*/,
         _b_close(false),_uid(0),_last_heartbeat(std::time(nullptr))
{
    // 创建一个uuid唯一标识session便于管理
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    _session_id = boost::uuids::to_string(a_uuid);
    _recv_head_node = std::make_shared<MsgNode>(HEAD_DATA_LEN);
}
boost::asio::ip::tcp::socket &Session::GetSocket()
{
    return _socket;
}
void Session::SetUid(int uid)
{
    _uid=uid;
}
int Session::GetUid()
{
    return _uid;
}
std::string &Session::GetSessionId()
{
    return _session_id;
}
Session::~Session()
{
    // 不用干什么 因为都用智能指针管理着资源
    //不在析构函数进行close关闭套接字资源 是为了防止服务端主动断开连接 造成大量TIME_WAIT状态
    std::cout<<_session_id<<"client close"<<std::endl;
    // 不需要关闭socket因为要销毁之前已经由上层调用了Close函数进行关闭
}

// 关闭socket套接字
void Session::Close()
{
    //服务端之间ctrl+c退出 不会显示调用socket的close
    //但是os检测到进程退出 会自动调用close关闭套接字资源 并发送fin包给客户端--此时是服务器主动断开连接
    std::cout<<"socket close: "<<_session_id<<std::endl;
    //多线程调用close
    // std::lock_guard<std::mutex> _lock(_mtx);
    if(_socket.is_open()){
    //即使 socket 被关闭，所有与该 socket 关联的未触发的异步操作的回调仍然会被调用
    //在回调中，error_code 参数会被设置为 boost::asio::error::operation_aborted，以表明操作被中止。这允许你在回调中区分正常完成和被取消的操作。
    _socket.close(); // 1.从io_context中移除 2.关闭文件描述符
    }
    _b_close = true;
}
void Session::HandleReadData(const boost::system::error_code &error, size_t bytes_transferred, std::shared_ptr<Session> self_ptr)
{
    try
    {
        if (!error.value())
        {
            //首先判断对方是不是合法用户
            if(!_server->CheckValid(_session_id))
            { //服务端已经踢了但是客户端不下线仍发消息
                Close();                      // 主动关闭socket--TIME_WAIT
                Session::DealExceptionSession();//清理登录信息  
                return;              
            }
            //更新时间戳
            UpdateHeartbeat();
            // 读取包体长度成功 形成逻辑包 扔到逻辑队列
            memcpy(_recv_data_node->_data,_data,_recv_data_node->_total_len);
            auto logic_msg = std::make_shared<LogicNode>(self_ptr, _recv_data_node);
            LogicSystem::GetInstance()->SetServer(_server);
            LogicSystem::GetInstance()->PostMsgToQue(std::move(logic_msg));
            // 继续进行读取头部数据
            _recv_head_node->Clear();
            Session::AsyncReadFull(HEAD_TOTAL_LEN,
                                    std::bind(&Session::HandleReadHead, this, std::placeholders::_1, std::placeholders::_2,
                                              shared_from_this()));
        }
        else
        {
            // 读取错误 可能对方断开连接 需要销毁连接 减少智能指针引用计数 ---主动断开 或者 被踢
            std::cout << "Read cilent error uuid: " << _session_id << std::endl;
            //此时可能是客户端正常退出 也有可能是被踢下去--- 会涉及到redis中连接数和连接所属ip的修改 需要添加分布式锁
            Close();                      // 被动关闭socket
            Session::DealExceptionSession();//清理登录信息
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
            //首先判断对方是不是合法用户
            if(!_server->CheckValid(_session_id))
            { //服务端已经踢了但是客户端不下线仍发消息
                Close();                      // 主动关闭socket--TIME_WAIT
                Session::DealExceptionSession();//清理登录信息
                return;                
            }           
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
                _server->ClearSession(_session_id); // 销毁socket结构
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
            std::cout << "Read cilent error uuid: " << _session_id << std::endl;
            //此时可能是客户端正常退出 也有可能是被踢下去--- 会涉及到redis中连接数和连接所属ip和sessionid的修改 需要添加分布式锁
            Close();                      // 关闭socket
            Session::DealExceptionSession();//清理登录信息--客户端已经主动断开连接
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
            //首先判断对方是不是合法用户
            // if(!_server->CheckValid(_session_id))
            // { //服务端已经踢了但是客户端不下线仍发消息
            //     Close();                      // 主动关闭socket--TIME_WAIT
            //     Session::DealExceptionSession();//清理登录信息                
            // }
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
            std::cout << "Send cilent error uuid: " << _session_id << std::endl;
            Close();                      // 关闭socket
            Session::DealExceptionSession();//清理登录信息
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
            std::cout << "session: " << _session_id << " send queue fulled" << std::endl;
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

void Session::NotifyOffline(int uid)
{ //向客户端发送下线请求 由客户端主动断开连接 服务端进行被动处理 防止出现大量TIME_wait状态
    nlohmann::json root;
    root["uid"]=uid;
    root["error"]=ErrorCodes::Success;
    std::string root_str=root.dump(4);
    Session::Write(MSGID_NOTIFY_OFF_LINE,root_str.length(),root_str.c_str());
}

//客户端被踢或者主动断开链接之后 对登录信息的处理 需要和登录时加同一把用户级别锁 保证同一用户登录信息的互斥访问
void Session::DealExceptionSession()
{   
    int uid=_uid;
    {
    //加用户级分布式锁
     std::string identifer_user = DistLock::GetInstance()->acquireLock(std::to_string(uid), LOCKTIMEOUT, ACQUIRETIME);
        if (identifer_user == "")
        { // 获取用户锁失败
            return;
        }
        Defer defer_userlock([this,identifer_user, uid]() { // 出{}自动解锁
            _server->ClearSession(_session_id); //这里对内存中存储的session进行清除--会加一把session的线程锁
            DistLock::GetInstance()->releaseLock(std::to_string(uid), identifer_user);
        });
        mINI::INIFile file("../conf/config.ini");
        mINI::INIStructure ini;
        file.read(ini);
        auto redis_con = RedisMgr::GetInstance()->GetRedisCon();
        // 1.设置连接数减少1写入redis ---加一把LOCK_COUNT的锁 -----保证这个登录数在分布式架构中读写是安全的
        //这里不在每次删除一个连接的时候进行-1连接数 而是由一个单独的心跳检测机制定时去更新最新的连接数  提高服务器的性能
        // {
        //     std::string identifer_count = DistLock::GetInstance()->acquireLock(LOCK_COUNT, LOCKTIMEOUT, ACQUIRETIME);
        //     Defer defer_countlock([identifer_count]() { // 出{}自动解锁
        //         DistLock::GetInstance()->releaseLock(LOCK_COUNT, identifer_count);
        //     });
        //     auto ret_redis = redis_con->hget(LOGIN_COUNT, ini["SelfServer"]["name"].c_str());
        //     if (!ret_redis.has_value())
        //     { // redis中没有这个chatServer的连接数缓存 服务器没有启动 基本不存在这个可能
        //         return;
        //     }
        //     else
        //     { // 更新连接数-1并写入redis
        //         redis_con->hset(LOGIN_COUNT, ini["SelfServer"]["name"].c_str(), std::to_string((atoi(ret_redis.value().c_str()) - 1)));
        //     }
        // }
        // 6.将uid和所在的服务器名称redis从redis删除  &&   将uid对应的sessionid绑定关系从redis删除
        //删除前先去判断是不是已经被新连接修改 如果修改则不进行删除 --判断的依据不是ip 而是sessionid（具有唯一性）
        std::string session_key = USERSESSIONIDPREFIX; // usessionid_1
        session_key += std::to_string(uid);
        std::string ip_key=USERIPPREFIX;  //uip_1
        ip_key+=std::to_string(uid);
        auto ret=redis_con->hget(UID_SESSIONS, session_key.c_str());
        if(ret.has_value())
        { //找到sessionid
            if(ret.value()!=_session_id)
            { //不相等 说明先登录再下线
                std::cout<<"新用户先登录 老用户后下线"<<std::endl;
                return;}
            //相等 说明先下线再登录 --需要进行删除redis中旧的登录信息
            redis_con->hdel(UID_SESSIONS,session_key.c_str());
            redis_con->hdel(UID_IPS,ip_key.c_str());
        }
    }
}

void Session::UpdateHeartbeat() //更新时间戳
{
    _last_heartbeat=std::time(nullptr);
}
bool Session::IsHeartbeatExpired() //判断是否超时
{
    time_t now=std::time(nullptr);
    double deff_time=std::difftime(now,_last_heartbeat);
    if(deff_time>EXPIRED_TIME)
    { //超时
        return true;
    }
    return false;
}