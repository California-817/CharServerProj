#pragma once
#include"grpc_cpp_out/GateServer.Status.pb.h"
#include"grpc_cpp_out/GateServer.Status.grpc.pb.h"
#include"mini/ini.h"
#include <grpcpp/grpcpp.h>
#include <iostream>
#include <memory>
#include <string>
#include<sstream>
#include<vector>
#include<mutex>
#include<condition_variable>
#include<queue>
#include<unordered_map>
#include <thread>
#include<boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include<climits>
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
	UidInvalid = 1011,  //uid无效
};
class ChatServer_struct //聊天服务器的结构体
{
public:
    ChatServer_struct(const std::string& host,const std::string& port,const std::string& name,const int& con_count)
    :_host(host),_port(port),_name(name),_con_count(con_count)
    {}
    std::string _host;
    std::string _port;
    std::string _name;
    int _con_count; //连接数量
};

#define USERIPPREFIX  "uip_"
#define USERTOKENPREFIX  "utoken_"
#define IPCOUNTPREFIX  "ipcount_"
#define USER_BASE_INFO "ubaseinfo_"
#define LOGIN_COUNT  "logincount"
#define UID_TOKENS "uid_tokens"
#define LOCK_PREFIX "lock:" //用户级锁的key前缀
#define LOCK_COUNT "lock_count" //读写连接数的系统级锁
#define LOCKTIMEOUT 10 //锁存活时间 防止死锁
#define ACQUIRETIME 5 //获取锁的时间
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