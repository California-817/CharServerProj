#include<iostream>
#include"../include/CServer.h"

int main()
{
    try{
        mINI::INIFile file("../conf/config.ini");
        mINI::INIStructure ini;
        file.read(ini);
        u_int16_t port=static_cast<uint16_t>(atoi(ini["GateServer"]["port"].c_str()));
        boost::asio::io_context io_context{1};
        boost::asio::signal_set signals(io_context,SIGINT,SIGTERM);
        signals.async_wait([&io_context](boost::system::error_code ec, int n){  //void (boost::system::error_code, int)
            if(ec)
            {
                return;}
            io_context.stop();
            std::cout<<"signal stop io_context"<<std::endl;
        });
        auto server=std::make_shared<CServer>(io_context,port);
        server->start();
        std::cout<<"GateServer listen on port :"<<port<<std::endl;
        io_context.run();//开启事件循环
    }
    catch(boost::system::system_error& err)
    {
        std::cout<<"err "<<err.what()<<std::endl;
    }
    return 0;
}