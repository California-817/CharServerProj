#pragma once
#include "Singleton.h"
#include "MsgNode.h"
#include "Session.h"
#include"Const.h"
#include"UserMgr.h"
#include"RedisMgr.h"
#include"MysqlMgr.h"
#include"LoginGrpcClient.h"
#include"ChatGrpcClient.h"
// 这是CRTP编程方式 这种编程方式 可以让父类对象访问子类对象的成员函数和成员变量（模板参数） 就好像是自己的类一样
// 可以用来实现静态多态（不同的子类对于父类不同模板参数 从而调用不同子类的函数实现多态） 避免动态多态的虚函数开销
class Session;
class LogicNode;
using FunCallback = std::function<void(std::shared_ptr<Session> session, const uint16_t &msg_id, const std::string &msg_data)>;
class LogicSystem : public Singleton<LogicSystem>
{
public:
    // 网络单元向逻辑队列抛任务
    void PostMsgToQue(std::shared_ptr<LogicNode> msg);
    ~LogicSystem();
public: //为啥声明基类是他的友元类还是访问不了私有成员
    // 处理逻辑任务
    void DealMsg();
    // 1.构造函数
    LogicSystem();
    // 逻辑单元自带的基本func
    void HelloWorldCallback(std::shared_ptr<Session> session, const uint16_t &msg_id, const std::string &msg_data);
    //登录回调
    void LoginCallback(std::shared_ptr<Session> session, const uint16_t &msg_id, const std::string &msg_data);
    //搜索好友回调
    void SearchCallback(std::shared_ptr<Session> session, const uint16_t &msg_id, const std::string &msg_data);
    //添加好友回调
    void AddFriendCallback(std::shared_ptr<Session> session, const uint16_t &msg_id, const std::string &msg_data);
    //认证好友回调
    void AuthFriendCallback(std::shared_ptr<Session> session, const uint16_t &msg_id, const std::string &msg_data);
    //文本信息发送回调
    void TextChatCallback(std::shared_ptr<Session> session, const uint16_t &msg_id, const std::string &msg_data);
    // 注册消息id和对应回调函数的映射关系
    void RegisterCallBacks();
    bool IsPureDigit(std::string);
    LogicSystem(const LogicSystem &) = delete;
    LogicSystem &operator=(const LogicSystem &) = delete;
private:
    bool GetUserInfo(int uid,UserInfo& userinfo);
    bool SearchInfoByUid(int uid,UserInfo& userinfo);
    bool SearchInfoByName(std::string name,UserInfo& userinfo);
private:
    std::thread _work_thread;                          // 工作线程 进行逻辑处理
    std::queue<std::shared_ptr<LogicNode>> _logic_que; // 逻辑队列
    std::mutex _mtx;
    std::condition_variable _cv;
    std::condition_variable _pv;
    std::atomic<bool> _b_stop;
    std::unordered_map<uint16_t, FunCallback> _fun_callbacks; //注册消息id和对应函数的映射关系
};