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
#include <boost/uuid/uuid_generators.hpp>
#include <arpa/inet.h>
#include <iostream>
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
};

//每一种消息对应一个数字 从而对应一个函数完成路由 
enum MSGID
{
    MSGID_HELLOWORLD=1001,
    MSGID_CHAT_LOGIN=1005, //登录的请求id
    MSGID_CHAT_LOGIN_RSP=1006, //登录的回应
};

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