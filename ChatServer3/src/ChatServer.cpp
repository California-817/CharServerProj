#include <iostream>
#include "../include/Server.h"
#include"../include/mini/ini.h"
#include"../include/RedisMgr.h"
#include"../include/ChatGrpcServerImpl.h"
int main()
{
    mINI::INIFile file("../conf/config.ini");
    mINI::INIStructure ini;
    file.read(ini);
    uint16_t port=static_cast<uint16_t>(atoi(ini["SelfServer"]["port"].c_str()));
    auto redis_con=RedisMgr::GetInstance()->GetRedisCon();
    //开启服务器自动设置连接数为0--加一把系统级的连接数分布式锁
    {
         std::string identifer_count = DistLock::GetInstance()->acquireLock(LOCK_COUNT, LOCKTIMEOUT, ACQUIRETIME);
        redis_con->hset(LOGIN_COUNT,ini["SelfServer"]["name"].c_str(),std::to_string(0));
         Defer defer_countlock([identifer_count]() { // 出{}自动解锁
                 DistLock::GetInstance()->releaseLock(LOCK_COUNT, identifer_count);
             });
    }
    try
    {
        //这个io_context只用来等待信号事件和accecpt事件的就绪--主线程处理
        std::shared_ptr<boost::asio::io_context> io_context(new boost::asio::io_context());
        // 注册信号事件 用于服务器优雅退出
        boost::asio::signal_set sigs(*io_context, SIGINT, SIGTERM);
        //开启grpc的服务端用于chatserver之间的通信  --多个子线程
        uint16_t grpc_server_port=static_cast<uint16_t>(atoi(ini["SelfServer"]["rpcport"].c_str()));
        Server ser(io_context, port);
        std::unique_ptr<ChatServerImpl> grpc_server=std::make_unique<ChatServerImpl>(&ser);
        grpc_server->Run(grpc_server_port);
        sigs.async_wait([&io_context,&grpc_server](auto, auto)
                        { 
                            std::cout<<"server quit"<<std::endl;
                            io_context->stop(); //主线程不会再次进行事件循环 走到程序结尾 清理资源
                            //进行grpc服务端的关闭
                            grpc_server->_server->Shutdown();
                            grpc_server->_cq->Shutdown();
                             });
        io_context->run(); // 等待事件循环 进行接收操作 调用回调函数 需要保证持续的有事件注册
    }
    catch (boost::system::system_error &exp)
    {
        std::cout << "file:" << __FILE__ << " " << "line" << __LINE__ << std::endl;
        std::cout << exp.what() << std::endl;
        std::cout << exp.code().value() << std::endl;
    }
    //结束时删除这个server的连接数
    //加一把系统级的连接数分布式锁
    {
         std::string identifer_count = DistLock::GetInstance()->acquireLock(LOCK_COUNT, LOCKTIMEOUT, ACQUIRETIME);
        redis_con->hdel(LOGIN_COUNT,ini["SelfServer"]["name"].c_str());
         Defer defer_countlock([identifer_count]() { // 出{}自动解锁
                 DistLock::GetInstance()->releaseLock(LOCK_COUNT, identifer_count);
             });
    }
    //所有局部对象的析构函数都会在主线程（即运行 main() 函数的线程）中执行
    return 0; //这里会进行所有变量的析构 包括static对象  ----前提是程序正常退出 优雅退出
}
