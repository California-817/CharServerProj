#pragma once  //存放公共头文件和各种enum字段
#include<iostream> 
#include<boost/beast/http.hpp>
#include<boost/beast.hpp>
#include<boost/asio.hpp>
#include<json/json.h> 
#include<mutex>
#include<string>
#include<functional>
#include<boost/uuid/uuid.hpp>
#include<boost/uuid/uuid_generators.hpp>
#include<boost/uuid/uuid_io.hpp>
#include<memory>
#include<chrono>
#include<unordered_map>
#include<queue>
#include<vector>
#include<atomic>
#include<condition_variable>
#include<thread>
#include"nlohmann/json.hpp"
#include"mini/ini.h"
//post请求的正文json数据对应error的各种值
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