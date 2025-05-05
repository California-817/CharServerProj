#pragma once
#include"grpc_cpp_out/GateServer.Status.pb.h"
#include"grpc_cpp_out/GateServer.Status.grpc.pb.h"
#include"mini/ini.h"
#include <grpcpp/grpcpp.h>
#include <iostream>
#include <memory>
#include <string>
#include<vector>
#include<mutex>
#include <thread>
#include<boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>


struct Generate_unique_string
{
    std::string operator()()
    {
         // 创建UUID对象
        boost::uuids::uuid uuid = boost::uuids::random_generator()();
        // 将UUID转换为字符串
        std::string unique_string = to_string(uuid);
         return unique_string;
    }
};
struct ChatServer //聊天服务器的属性
{
    ChatServer(const std::string& host,const std::string& port)
    :_host(host),_port(port)
    {}
    std::string _host;
    std::string _port;
};