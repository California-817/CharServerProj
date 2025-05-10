#include "../include/LogicSystem.h"
// 网络单元向逻辑队列抛任务
void LogicSystem::PostMsgToQue(std::shared_ptr<LogicNode> msg)
{
    std::unique_lock<std::mutex> _ulk(_mtx);
    while (_logic_que.size() >= MAX_RECVLEN) // 防止伪唤醒
    {
        // 逻辑队列的大小到达限制 等待50ms 如果超时直接丢包
        std::cv_status ret = _pv.wait_for(_ulk, std::chrono::milliseconds(50));
        if (ret == std::cv_status::timeout)
        {
            // 等待超时了
            if (_logic_que.size() >= MAX_RECVLEN)
            {
                // 队列没有空间 丢包并打印日志
                std::cout << "client " << msg->GetSession()->GetUuid() << " data has been dropped because the Logicqueue is fulled"
                          << std::endl;
                return;
            }
            // 超时但是队列有空间
        }
        // 未超时
    }
    // 走到这一定有空间
    _logic_que.push(std::move(msg));
    // 里面有任务了 唤醒消费线程
    _cv.notify_one();
}

// 处理逻辑任务
void LogicSystem::DealMsg() // 线程的工作函数
{
    for (;;)
    {

        std::unique_lock<std::mutex> _ulk(_mtx);
        while (_logic_que.empty() && !_b_stop) // 没数据 && 没stop逻辑线程 都满足才会等待
        {
            // 队列为空
            _cv.wait(_ulk);
        }
        // 有一个不满足就跳出
        // 1._b_stop设置为true表示关服
        if (_b_stop)
        {
            // 关服逻辑处理 处理完全部数据
            while (!_logic_que.empty())
            {
                auto logicnode = _logic_que.front();
                _logic_que.pop();
                auto it = _fun_callbacks.find(logicnode->GetRecvNode()->_msg_id);
                if (it == _fun_callbacks.end()) // 没找到id对应函数
                {
                    continue;
                }
                it->second(std::move(logicnode->GetSession()), logicnode->GetRecvNode()->_msg_id,
                           std::string(logicnode->GetRecvNode()->_data, logicnode->GetRecvNode()->_total_len));
            }
            break;
        }

        // 2.未关服
        auto logicnode = _logic_que.front();
        _logic_que.pop();
        _pv.notify_one();//有空间唤醒消费者线程

        _ulk.unlock(); //以下是进行逻辑处理 不涉及访问临界资源 直接解锁
        auto it = _fun_callbacks.find(logicnode->GetRecvNode()->_msg_id);
        if (it == _fun_callbacks.end()) // 没找到id对应函数
        {
            continue;
        }
        it->second(std::move(logicnode->GetSession()), logicnode->GetRecvNode()->_msg_id,
                   std::string(logicnode->GetRecvNode()->_data, logicnode->GetRecvNode()->_total_len));
    }
}
// 1.构造函数
LogicSystem::LogicSystem()
    : Singleton<LogicSystem>(), _b_stop(false)
{
    // 开始期间注册最基本的id和对应处理函数
    RegisterCallBacks();
    // 工作线程不断处理逻辑任务
    _work_thread = std::thread(&LogicSystem::DealMsg, this);
}
// 注册消息id和对应回调函数的映射关系
void LogicSystem::RegisterCallBacks()
{
    _fun_callbacks[MSGID_HELLOWORLD] = std::bind(&LogicSystem::HelloWorldCallback, this,
                                                 std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    _fun_callbacks[MSGID_CHAT_LOGIN]=std::bind(&LogicSystem::LoginCallback,this,
                                                 std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

LogicSystem::~LogicSystem()
{
    //析构函数会进行关服操作
    _b_stop=true;
    _cv.notify_one(); //唤醒工作线程
    _work_thread.join(); //等待工作线程退出
    std::cout<<"logicSystem close"<<std::endl;
}

// 逻辑单元自带的基本func
void LogicSystem::HelloWorldCallback(std::shared_ptr<Session> session, const uint16_t &msg_id, const std::string &msg_data)
{
    std::cout << "recv msg from " << session->GetUuid() << " " << msg_data << std::endl;
    std::string resp = "logic handle msg and send respose";
    session->Write(msg_id, resp.size(), resp.c_str());
}

//登录逻辑的处理函数
void LogicSystem::LoginCallback(std::shared_ptr<Session> session, const uint16_t &msg_id, const std::string &msg_data)
{
    // jsonObj["uid"] = _uid;
    // jsonObj["token"] = _token;
    nlohmann::json root;
    nlohmann::json src_root;
    try{
        src_root=nlohmann::json::parse(msg_data);
    }catch(const nlohmann::json::parse_error& err)
    { //反序列化失败
      std::cout<<"Fail to parse json "<<err.what()<<std::endl;
      root["error"]=ErrorCodes::Error_Json;
      auto jsonstr=root.dump(4); //序列化
      session->Write(MSGID_CHAT_LOGIN_RSP,jsonstr.size(),jsonstr.c_str());
      return;
    }
    int uid=src_root["uid"].get<int>();
    std::string token=src_root["token"].get<std::string>();
    std::cout<<uid<<" : "<<token<<std::endl;
    //进行校验等处理
    LoginRsp ret=LoginGrpcClient::GetInstance()->Login(uid,token);
    if(ret.error()!=ErrorCodes::Success)
    { //校验错误
        root["error"]=ret.error();
        auto ret_str=root.dump(4);
        session->Write(MSGID_CHAT_LOGIN_RSP,ret_str.size(),ret_str.c_str());
        return;
    }
    //校验登录成功 发送响应
        //  auto uid = jsonObj["uid"].toInt();
        // auto name = jsonObj["name"].toString();
        // auto nick = jsonObj["nick"].toString();
        // auto icon = jsonObj["icon"].toString();
        // auto sex = jsonObj["sex"].toInt();
    root["error"]=ErrorCodes::Success;
    root["uid"]=uid;
    root["name"]="xxxten";
    root["nick"]="xxxten";
    root["sex"]=1;
    root["icon"]="";
    root["token"]=token;
    auto ret_str=root.dump(4);
    session->Write(MSGID_CHAT_LOGIN_RSP,ret_str.size(),ret_str.c_str());
}

