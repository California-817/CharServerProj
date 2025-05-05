#include"../include/StatusServerImpl.h"
int main()
{
    mINI::INIFile file("../conf/config.ini");
    mINI::INIStructure ini;
    file.read(ini);
    u_int16_t port=static_cast<uint16_t>(atoi(ini["StatusServer"]["port"].c_str()));
    StatusServerImpl my_server;
    my_server.Run(port);
    return 0;
}