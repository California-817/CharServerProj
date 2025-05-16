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
                std::cout << "client " << msg->GetSession()->GetSessionId() << " data has been dropped because the Logicqueue is fulled"
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
    _fun_callbacks[MSGID_SEARCH_USER]=std::bind(&LogicSystem::SearchCallback,this,
                                                 std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    _fun_callbacks[MSGID_ADD_FRIEND]=std::bind(&LogicSystem::AddFriendCallback,this,
                                                 std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);   
    _fun_callbacks[MSGID_AUTH_FRIEND]=std::bind(&LogicSystem::AuthFriendCallback,this,
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
    std::cout << "recv msg from " << session->GetSessionId() << " " << msg_data << std::endl;
    std::string resp = "logic handle msg and send respose";
    session->Write(msg_id, resp.size(), resp.c_str());
}

//登录逻辑的处理函数 -----未来会用到分布式锁来独占登录
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
    //1.进行校验等处理
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
    //2.从数据库获取用户信息
    UserInfo userinfo;
    if(!LogicSystem::GetUserInfo(uid,userinfo))
    { //获取信息失败
        root["error"]=ErrorCodes::UidInvalid;
        auto ret_str=root.dump(4);
        session->Write(MSGID_CHAT_LOGIN_RSP,ret_str.size(),ret_str.c_str());
        return;
    }
    //获取信息成功写入响应
    root["error"]=ErrorCodes::Success;
    root["uid"]=uid;
    root["name"]=userinfo._name;
    root["nick"]=userinfo._nick;
    root["sex"]=userinfo._sex;
    root["icon"]=userinfo._icon;
    root["pwd"]=userinfo._pwd;
    root["email"]=userinfo._email;
    root["desc"]=userinfo._desc;
    //3.加载申请列表 todo
    std::vector<std::shared_ptr<ApplyInfo>> applylist; //申请人列表
    if(MysqlMgr::GetInstance()->GetFriendApply(uid,applylist,0,10))
    { //成功获取并写入applylist
        nlohmann::json jsonarray;
        for(auto& apply: applylist)
        {
            nlohmann::json _apply_root;
            _apply_root["name"]=apply->_name;
            _apply_root["desc"]=apply->_desc;
            _apply_root["icon"]=apply->_icon;
            _apply_root["nick"]=apply->_nick;
            _apply_root["sex"]=apply->_sex;
            _apply_root["uid"]=apply->_uid;
            _apply_root["status"]=apply->_status;
            jsonarray.push_back(_apply_root);
        }
        //向响应中写入applylist
        root["apply_list"]=jsonarray;
    }
    //4.加载好友列表 todo
    std::vector<std::shared_ptr<UserInfo>> friendlist; //好友列表
    if(MysqlMgr::GetInstance()->GetFriendList(uid,friendlist))
    { //成功获取并写入applylist
        // auto name = value["name"].toString();
        // auto desc = value["desc"].toString();
        // auto icon = value["icon"].toString();
        // auto nick = value["nick"].toString();
        // auto sex = value["sex"].toInt();
        // auto uid = value["uid"].toInt();
        // auto back = value["back"].toString();
        nlohmann::json jsonarray;
        for(auto& frd: friendlist)
        {
            nlohmann::json _frd_root;
            _frd_root["name"]=frd->_name;
            _frd_root["desc"]=frd->_desc;
            _frd_root["icon"]=frd->_icon;
            _frd_root["nick"]=frd->_nick;
            _frd_root["sex"]=frd->_sex;
            _frd_root["uid"]=frd->_uid;
            _frd_root["back"]=frd->_back;
            jsonarray.push_back(_frd_root);
        }
        //向响应中写入applylist
        root["friend_list"]=jsonarray;
    }    
    //5.设置连接数增加写入redis
    auto redis_con=RedisMgr::GetInstance()->GetRedisCon();
    mINI::INIFile file("../conf/config.ini");
    mINI::INIStructure ini;
    file.read(ini);
    auto ret_redis=redis_con->hget(LOGIN_COUNT,ini["SelfServer"]["name"].c_str());
    if(!ret_redis.has_value()){  //redis中没有这个chatServer的连接数缓存 服务器没有启动 基本不存在这个可能
        root["error"]=ErrorCodes::UidInvalid;
        auto ret_str=root.dump(4);
        session->Write(MSGID_CHAT_LOGIN_RSP,ret_str.size(),ret_str.c_str());
        return;}
    else{ //更新连接数+1并写入redis
       redis_con->hset(LOGIN_COUNT,ini["SelfServer"]["name"].c_str(),std::to_string((atoi(ret_redis.value().c_str())+1)));}
    //6.将uid和所在的服务器名称写入redis
    std::string ip_key=USERIPPREFIX; // uip_1
    ip_key+=std::to_string(uid);
    redis_con->hset(UID_IPS,ip_key.c_str(),ini["SelfServer"]["name"].c_str());
    //7.本地绑定uid和session的映射关系
    session->SetUid(uid);
    UserMgr::GetInstance()->SetUidSession(uid,session);
    auto ret_str=root.dump(4);
    session->Write(MSGID_CHAT_LOGIN_RSP,ret_str.size(),ret_str.c_str());
}


bool LogicSystem::GetUserInfo(int uid,UserInfo& userinfo)
{
    //1.先去redis中获取用户信息
    auto redis_con=RedisMgr::GetInstance()->GetRedisCon();
    if(redis_con.get()==nullptr)
    {
        return false;}
    std::string info_key=USER_BASE_INFO;
    info_key+=std::to_string(uid);   // ubaseinfo_1
    // std::unordered_map<std::string,std::string> redis_ret;
    // auto redis_ret=redis_con->hgetall<std::unordered_map<std::string, std::string>>(info_key);
    // 用于存储结果的容器
    std::unordered_map<std::string, std::string> redis_ret;
    std::vector<std::string> fields{"uid","name","email","nick","pwd","desc","icon","sex"};
    // 对每个字段调用 hget
    for (const auto& field : fields) {
        auto value = redis_con->hget(info_key, field);
        if (value.has_value()) {
            redis_ret[field] = value.value();
        }
    }
    if(!redis_ret.empty())
    { //redis中有该用户信息的缓存
        userinfo._uid=atoi(redis_ret["uid"].c_str());
        userinfo._name=redis_ret["name"];
        userinfo._email=redis_ret["email"];
        userinfo._nick=redis_ret["nick"];
        userinfo._pwd=redis_ret["pwd"];
        userinfo._desc=redis_ret["desc"];
        userinfo._icon=redis_ret["icon"];
        userinfo._sex=atoi(redis_ret["sex"].c_str());
        return true;
    }
    //redis中没有数据缓存 到MySQL获取 并且写入redis
    bool ret=MysqlMgr::GetInstance()->GetUserInfo(uid,userinfo);
    if(ret)
    { //成功获取并写入了userinfo  写入redis  
        redis_con->hmset(info_key,{std::make_pair("uid", std::to_string(userinfo._uid).c_str()),
            std::make_pair("name", userinfo._name.c_str()),
            std::make_pair("nick", userinfo._nick.c_str()),
            std::make_pair("pwd", userinfo._pwd.c_str()),
            std::make_pair("desc", userinfo._desc.c_str()),
            std::make_pair("icon", userinfo._icon.c_str()),
            std::make_pair("sex", std::to_string(userinfo._sex).c_str()),
            std::make_pair("email", userinfo._email.c_str())});
        return true;
    }
    return false;
}
//搜索好友的请求
void LogicSystem::SearchCallback(std::shared_ptr<Session> session, const uint16_t &msg_id, const std::string &msg_data)
{
    //请求
	//jsonObj["uid"] = uid_str;  //uid或者名字
    nlohmann::json root; //回包
    nlohmann::json src_root;
    try{
        src_root=nlohmann::json::parse(msg_data);
    }catch(const nlohmann::json::parse_error& err)
    { //反序列化失败
      std::cout<<"Fail to parse json "<<err.what()<<std::endl;
      root["error"]=ErrorCodes::Error_Json;
      auto jsonstr=root.dump(4); //序列化
      session->Write(MSGID_SEARCH_USER_RSP,jsonstr.size(),jsonstr.c_str());
      return;
    }
    root["error"]=ErrorCodes::Success;
    Defer defer([&root,&session](){ 
        auto jsonstr=root.dump(4); //序列化
        session->Write(MSGID_SEARCH_USER_RSP,jsonstr.size(),jsonstr.c_str());
    });
    std::string uid=src_root["uid"].get<std::string>(); 
    //判断是不是纯数字
    bool is_digit=IsPureDigit(uid);
    UserInfo userinfo;
    if(is_digit)
    { //纯数字 uid 用uid查找用户信息
        bool ret=SearchInfoByUid(atoi(uid.c_str()),userinfo);
        if(!ret)
        {
            root["error"]=ErrorCodes::UidInvalid;
            return;
        }
    }
    else{ //通过名字查找用户信息
        bool ret=SearchInfoByName(uid,userinfo);
        if(!ret)
        {
            root["error"]=ErrorCodes::UidInvalid;
            return;
        }
    }
    //成功获取到对方的信息 进行输出
    //回包
    //jsonObj["error"].toInt()
    //jsonObj["uid"].toInt(), jsonObj["name"].toString(),
    //jsonObj["nick"].toString(), jsonObj["desc"].toString(),
    //jsonObj["sex"].toInt(), jsonObj["icon"].toString());
    root["uid"]=userinfo._uid;
    root["name"]=userinfo._name;
    root["nick"]=userinfo._nick;
    root["desc"]=userinfo._desc;
    root["sex"]=userinfo._sex;
    root["icon"]=userinfo._icon;
}
bool LogicSystem::IsPureDigit(std::string uid)
{
    for(auto e:uid)
    {
        if(!std::isdigit(e))
        {
            return false;
        }
    }
    return true;
}
bool LogicSystem::SearchInfoByUid(int uid,UserInfo& userinfo)
{   
    return LogicSystem::GetUserInfo(uid,userinfo);
}
bool LogicSystem::SearchInfoByName(std::string name,UserInfo& userinfo)
{
    //1.先去redis中获取用户信息
    auto redis_con=RedisMgr::GetInstance()->GetRedisCon();
    if(redis_con.get()==nullptr)
    {
        return false;}
    std::string info_key=USER_NAME;
    info_key+=name;   // user_name_xxxten
    // std::unordered_map<std::string,std::string> redis_ret;
    // auto redis_ret=redis_con->hgetall<std::unordered_map<std::string, std::string>>(info_key);
    // 用于存储结果的容器
    std::unordered_map<std::string, std::string> redis_ret;
    std::vector<std::string> fields{"uid","name","email","nick","pwd","desc","icon","sex"};
    // 对每个字段调用 hget
    for (const auto& field : fields) {
        auto value = redis_con->hget(info_key, field);
        if (value.has_value()) {
            redis_ret[field] = value.value();
        }
    }
    if(!redis_ret.empty())
    { //redis中有该用户信息的缓存
        userinfo._uid=atoi(redis_ret["uid"].c_str());
        userinfo._name=redis_ret["name"];
        userinfo._email=redis_ret["email"];
        userinfo._nick=redis_ret["nick"];
        userinfo._pwd=redis_ret["pwd"];
        userinfo._desc=redis_ret["desc"];
        userinfo._icon=redis_ret["icon"];
        userinfo._sex=atoi(redis_ret["sex"].c_str());
        return true;
    }
    //redis中没有数据缓存 到MySQL获取 并且写入redis
    bool ret=MysqlMgr::GetInstance()->GetUserInfo(name,userinfo);
    if(ret)
    { //成功获取并写入了userinfo  写入redis  
        redis_con->hmset(info_key,{std::make_pair("uid", std::to_string(userinfo._uid).c_str()),
            std::make_pair("name", userinfo._name.c_str()),
            std::make_pair("nick", userinfo._nick.c_str()),
            std::make_pair("pwd", userinfo._pwd.c_str()),
            std::make_pair("desc", userinfo._desc.c_str()),
            std::make_pair("icon", userinfo._icon.c_str()),
            std::make_pair("sex", std::to_string(userinfo._sex).c_str()),
            std::make_pair("email", userinfo._email.c_str())});
        return true;
    }
    return false;    
}

void LogicSystem::AddFriendCallback(std::shared_ptr<Session> session, const uint16_t &msg_id, const std::string &msg_data)
{
    mINI::INIFile file("../conf/config.ini");
    mINI::INIStructure ini;
    file.read(ini);
    //请求
    // jsonObj["uid"] = uid;
    // jsonObj["applyname"] = name;
    // jsonObj["bakname"] = bakname;
    // jsonObj["touid"] = _si->_uid;
    //给自己的响应
    //jsonObj["error"].toInt();
    nlohmann::json root; //回包
    nlohmann::json src_root;
    try{
        src_root=nlohmann::json::parse(msg_data);
    }catch(const nlohmann::json::parse_error& err)
    { //反序列化失败
      std::cout<<"Fail to parse json "<<err.what()<<std::endl;
      root["error"]=ErrorCodes::Error_Json;
      auto jsonstr=root.dump(4); //序列化
      session->Write(MSGID_ADD_FRIEND_RSP,jsonstr.size(),jsonstr.c_str());
      return;
    }
    root["error"]=ErrorCodes::Success;
    Defer defer([&root,&session](){
      auto jsonstr=root.dump(4); //序列化
      session->Write(MSGID_ADD_FRIEND_RSP,jsonstr.size(),jsonstr.c_str());
    });
    int from_uid=src_root["uid"].get<int>();
    int to_uid=src_root["touid"].get<int>();
    std::string applyname=src_root["applyname"].get<std::string>();
    std::string bakname=src_root["bakname"].get<std::string>();
    //更新数据库的申请好友表
    bool insert_ret=MysqlMgr::GetInstance()->AddFriendApply(from_uid,to_uid);
    if(!insert_ret)
    { //插入记录失败
        root["error"]=ErrorCodes::UidInvalid;
        return;
    }
    //将申请者的基本信息获取
    std::shared_ptr<UserInfo> userinfo=std::make_shared<UserInfo>();
    bool info_get=LogicSystem::GetUserInfo(from_uid,*(userinfo.get()));
    if(!info_get)
    { //基本信息获取失败
        return;
    }
    //向对方发送通知好友申请的请求
    //0.查看对方的ip
    std::string to_uid_str=USERIPPREFIX;
    to_uid_str+=std::to_string(to_uid); //uip_1
    auto redis_con=RedisMgr::GetInstance()->GetRedisCon();
    auto redis_ret=redis_con->hget(UID_IPS,to_uid_str);
    if(!redis_ret.has_value())
    {
        return;        
    }
    std::string touid_ip=redis_ret.value();
    //给对方的请求
    //jsonObj["error"].toInt();
    //jsonObj["applyuid"].toInt();
    //jsonObj["name"].toString();
    //jsonObj["desc"].toString();
    //jsonObj["icon"].toString();
    //jsonObj["nick"].toString();
    //jsonObj["sex"].toInt();
    //1.对方在本服务器上
    if(touid_ip==ini["SelfServer"]["name"])
    {
        //找到对方的session
        auto to_session=UserMgr::GetInstance()->GetUidSession(to_uid);
        if(to_session.get()){ //对方在线
        nlohmann::json to_root;
        to_root["error"]=ErrorCodes::Success;
        to_root["applyuid"]=from_uid;
        to_root["name"]=userinfo->_name;
        to_root["desc"]=userinfo->_desc;
        to_root["icon"]=userinfo->_icon;
        to_root["nick"]=userinfo->_nick;
        to_root["sex"]=userinfo->_sex;
        std::string to_jsonstr=to_root.dump(4);
        //给对方发送申请好友请求
        to_session->Write(MSGID_NOTIFY_ADD_FRIEND,to_jsonstr.size(),to_jsonstr.c_str());
        }
        //对方不在线
        return;
    }
    //2.对方在其他服务器上 --调用grpc的申请好友服务
    AddFriendReq req;
    req.set_applyuid(from_uid);
    req.set_touid(to_uid);
    req.set_desc(userinfo->_desc);
    req.set_icon(userinfo->_icon);
    req.set_name(userinfo->_name);
    req.set_nick(userinfo->_nick);
    req.set_sex(userinfo->_sex);
    ChatGrpcClient::GetInstance()->NotifyAddFriend(touid_ip,req);
}


void LogicSystem::AuthFriendCallback(std::shared_ptr<Session> session, const uint16_t &msg_id, const std::string &msg_data)
{
    mINI::INIFile file("../conf/config.ini");
    mINI::INIStructure ini;
    file.read(ini);
    //请求
    // jsonObj["fromuid"] = uid; 
    // jsonObj["touid"] = _apply_info->_uid;
    // jsonObj["back"] = back_name;
    nlohmann::json root; //回包
    nlohmann::json src_root;
    try{
        src_root=nlohmann::json::parse(msg_data);
    }catch(const nlohmann::json::parse_error& err)
    { //反序列化失败
      std::cout<<"Fail to parse json "<<err.what()<<std::endl;
      root["error"]=ErrorCodes::Error_Json;
      auto jsonstr=root.dump(4); //序列化
      session->Write(MSGID_AUTH_FRIEND_RSP,jsonstr.size(),jsonstr.c_str());
      return;
    }
    root["error"]=ErrorCodes::Success;
    Defer defer([&session,&root](){
      auto jsonstr=root.dump(4); //序列化
      session->Write(MSGID_AUTH_FRIEND_RSP,jsonstr.size(),jsonstr.c_str());    
    });
    int from_uid=src_root["fromuid"].get<int>();
    int to_uid=src_root["touid"].get<int>();
    std::string back=src_root["back"].get<std::string>();
    UserInfo userinfo;
    bool ret=LogicSystem::GetUserInfo(to_uid,userinfo);
    if(!ret) //查找对方信息失败
    {
        root["error"]=ErrorCodes::UidInvalid;
        return;
    }
    root["name"]=userinfo._name;
    root["nick"]=userinfo._nick;
    root["icon"]=userinfo._icon;
    root["sex"]=userinfo._sex;
    root["uid"]=userinfo._uid;
    //回复（给自己）
    // int err = jsonObj["error"].toInt();
    // auto name = jsonObj["name"].toString();
    // auto nick = jsonObj["nick"].toString();
    // auto icon = jsonObj["icon"].toString();
    // auto sex = jsonObj["sex"].toInt();
    // auto uid = jsonObj["uid"].toInt();
    //更新mysql中的好友申请表 和 好友列表 -- 以事务的形式
    bool mysql_ret=MysqlMgr::GetInstance()->AuthAddFriend(from_uid,to_uid,back);
    if(!mysql_ret)
    {
        root["error"]=ErrorCodes::UidInvalid;
        return;       
    }
    //给对方发送认证好友请求
    //查找对方所在的服务器
    std::string to_uid_str=USERIPPREFIX;
    to_uid_str+=std::to_string(to_uid); //uip_1
    auto redis_con=RedisMgr::GetInstance()->GetRedisCon();
    auto redis_ret=redis_con->hget(UID_IPS,to_uid_str);
    if(!redis_ret.has_value())
    { //对方没有上线
        return;        
    }
    std::string touid_ip=redis_ret.value();
    //1.对方在本服务器上
    if(touid_ip==ini["SelfServer"]["name"])
    {
        //找到对方的session
        auto to_session=UserMgr::GetInstance()->GetUidSession(to_uid);
        if(to_session.get()){ //对方在线
        //将我的信息发送给对方
        UserInfo userinfo;
        LogicSystem::GetUserInfo(from_uid,userinfo);     
        nlohmann::json to_root;
        to_root["error"]=ErrorCodes::Success;
        to_root["fromuid"]=userinfo._uid;
        to_root["name"]=userinfo._name;
        to_root["nick"]=userinfo._nick;
        to_root["icon"]=userinfo._icon;
        to_root["sex"]=userinfo._sex;
        std::string to_jsonstr=to_root.dump(4);
        //给对方发送申请好友请求
        to_session->Write(MSGID_NOTIFY_AUTH_FRIEND,to_jsonstr.size(),to_jsonstr.c_str());
        }
        //对方不在线
        return;
    }
    //2.对方在其他服务器上 --调用grpc的申请好友服务
    AuthFriendReq req;
    req.set_fromuid(from_uid);
    req.set_touid(to_uid);
    ChatGrpcClient::GetInstance()->NotifyAuthFriend(touid_ip,req);
    //通知对方认证好友请求（给对方）
    // int err = jsonObj["error"].toInt();
    // int from_uid = jsonObj["fromuid"].toInt();
    // QString name = jsonObj["name"].toString();
    // QString nick = jsonObj["nick"].toString();
    // QString icon = jsonObj["icon"].toString();
    // int sex = jsonObj["sex"].toInt();
}





