#pragma once
#include<thread>
#include<vector>
#include<memory>
#include<boost/asio.hpp>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include<mutex>
#include<chrono>
#include <atomic>
#include <cstring>
#include <boost/uuid/uuid_io.hpp>
#include<cctype>
#include <boost/uuid/uuid_generators.hpp>
#include <arpa/inet.h>
#include"grpc_cpp_out/ChatServer.Chat.pb.h"
#include"grpc_cpp_out/ChatServer.Chat.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <iostream>
#include<mysql.h>
#include"nlohmann/json.hpp"
#include"mini/ini.h"
#include <functional>
//单个value的最大长度
#define MAX_LENGTH 1024*2
//头部总长度
#define HEAD_TOTAL_LEN 4
//头部id长度
#define HEAD_ID_LEN 2
//头部length长度
#define HEAD_DATA_LEN 2
//接收队列的最大大小
#define MAX_RECVLEN 10000
//每个session的发送queue的最大长度
#define MAX_SENDLEN 1000
enum ErrorCodes
{
    Success = 0,  //成功
    Error_Json = 1001,  //Json解析错误
    RPCFailed = 1002,  //RPC请求错误
    VarifyExpired=1003, //验证码过期
    VarifyCodeErr=1004, //验证码错误
    UserExist=1005,  //用户已经存在
    EmailExist=1010, //邮箱已经存在
    PassWordErr=1006, //密码错误
    EmailNotMatch=1007, //邮箱不匹配
    PassWordUpErr=1008,//密码更新失败
    PassWordInvalid=1009,//密码无效
    TokenInvalid = 1010,   //Token无效
	UidInvalid = 1011, //uid无效
};

//每一种消息对应一个数字 从而对应一个函数完成路由 
enum MSGID
{
    MSGID_HELLOWORLD=1001,
    MSGID_CHAT_LOGIN=1005, //登录的请求id
    MSGID_CHAT_LOGIN_RSP=1006, //登录的回应
    MSGID_SEARCH_USER=1007, //搜索好友请求
    MSGID_SEARCH_USER_RSP=1008, //搜索好友响应
    MSGID_ADD_FRIEND=1009,  //添加好友请求
    MSGID_ADD_FRIEND_RSP=1010, //添加好友回复
    MSGID_NOTIFY_ADD_FRIEND=1011, //通知添加好友请求
    MSGID_AUTH_FRIEND=1013,//认证好友请求
    MSGID_AUTH_FRIEND_RSP=1014,//认证好友回复
    MSGID_NOTIFY_AUTH_FRIEND=1015,//通知认证好友请求
    MSGID_TEXT_CHAT=1017, //文本聊天信息请求
    MSGID_TEXT_CHAT_RSP=1018, //文本聊天信息回复
    MSGID_NOTIFY_TEXT_CHAT=1019, //文本聊天信息请求
    MSGID_NOTIFY_OFF_LINE = 1021, //通知用户下线
    MSGID_HEARTBEAT=1023, //心跳包请求
    MSGID_HEARTBEAT_RSP=1024,//心跳包响应
};
#define USERSESSIONIDPREFIX "usessionid_" //用户sessionid的key值
#define USERIPPREFIX  "uip_"  //用户所在的ip的uidkey值
#define USER_BASE_INFO "ubaseinfo_" //一个hash名称 存放一个用户的基本信息 通过uid
#define USER_NAME "user_name_"  //用于查找用户通过名字查找存放用户信息
#define LOGIN_COUNT  "logincount" //一个hash名称 存放所有服务器的连接数
#define UID_TOKENS "uid_tokens" //一个hash名称 存放所有uid以及tokens
#define UID_IPS "uid_at_ip"   //一个hash名称 存放所有uid以及所在服务器的名称
#define UID_SESSIONS "uid_bind_sessionid" //存放所有uid对应的session的id
#define LOCK_PREFIX "lock:" //用户级锁的key前缀
#define LOCK_COUNT "lock_count" //读写连接数的系统级锁
#define LOCKTIMEOUT 10 //锁存活时间 防止死锁
#define ACQUIRETIME 5 //获取锁的时间
#define CHECK_TIMEOUT 60 //每隔60s检测连接超时
#define EXPIRED_TIME 20 //20s作为超时时间 20s内无消息接收就认为连接异常 断开连接
class Defer
{  //实现类似go的defer 当出了函数作用域 对象销毁自动调用析构函数 在析构函数内部调用外部需要执行的操作_func
public:
    Defer(std::function<void()> func)
    :_func(func){}
    ~Defer()
    {
        _func();
    }
private:
    std::function<void()> _func;
};
 
//用户信息的结构体
struct UserInfo
{
    UserInfo(int uid,std::string name,std::string email,std::string pwd,std::string nick,std::string desc,std::string icon,int sex,std::string back)
    :_uid(uid),_name(name),_email(email),_pwd(pwd),_desc(desc),_icon(icon),_sex(sex),_nick(nick),_back(back){}
    UserInfo(){};
    int _uid;
    std::string _name;
    std::string _nick;
    std::string _email;
    std::string _pwd;
    std::string _desc;
    std::string _icon; //头像
    int _sex; //性别
    std::string _back; //备注名 用于好友列表
};
//申请人结构体
struct ApplyInfo
{
    ApplyInfo(int uid,std::string name,std::string nick,std::string desc,std::string icon,int sex,int status)
    :_uid(uid),_name(name),_desc(desc),_icon(icon),_sex(sex),_nick(nick),_status(status){}
    ApplyInfo(){};
	int _uid;
	std::string _name;
    std::string _nick;
	std::string _desc;
	std::string _icon;
	int _sex;
	int _status; //是否已经添加
};