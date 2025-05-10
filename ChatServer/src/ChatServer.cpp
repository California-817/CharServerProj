#include <iostream>
#include "../include/Server.h"
#include"../include/mini/ini.h"
int main()
{
    mINI::INIFile file("../conf/config.ini");
    mINI::INIStructure ini;
    file.read(ini);
    uint16_t port=static_cast<uint16_t>(atoi(ini["SelfServer"]["port"].c_str()));
    try
    {
        //这个io_context只用来等待信号事件和accecpt事件的就绪--主线程处理
        std::shared_ptr<boost::asio::io_context> io_context(new boost::asio::io_context());
        // 注册信号事件 用于服务器优雅退出
        boost::asio::signal_set sigs(*io_context, SIGINT, SIGTERM);
        sigs.async_wait([&io_context](auto, auto)
                        { 
                            std::cout<<"server quit"<<std::endl;
                            io_context->stop(); });
        Server ser(io_context, port);
        io_context->run(); // 等待事件循环 进行IO操作 调用回调函数 需要保证持续的有事件注册
    }
    catch (boost::system::system_error &exp)
    {
        std::cout << "file:" << __FILE__ << " " << "line" << __LINE__ << std::endl;
        std::cout << exp.what() << std::endl;
        std::cout << exp.code().value() << std::endl;
    }
    return 0;
}