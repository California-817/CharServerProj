#include"../include/StatusServerImpl.h"
using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using GateServer::Status::GetChatServerReq;
using GateServer::Status::GetChatServerRsp;
using GateServer::Status::StatusServer;
StatusServerImpl::StatusServerImpl()
    :_server_index(0){
        std::string host="127.0.0.1";
        std::string port1="8083";
        std::string port2="8084";
        _chat_servers.emplace_back(host,port1);
        _chat_servers.emplace_back(host,port2);
    }
void StatusServerImpl::Run(uint16_t port)
    {
        std::string server_address="0.0.0.0:8082";
        ServerBuilder builder;
        builder.AddListeningPort(server_address,grpc::InsecureServerCredentials());
        builder.RegisterService(&_service);
        _cq=builder.AddCompletionQueue();
        _server=builder.BuildAndStart();
        std::cout << "Server listening on " << server_address << std::endl;
        for(int i=0;i<std::thread::hardware_concurrency();i++)
        {
            _threads.emplace_back(&StatusServerImpl::HandleRpcs,this);
        }
        boost::asio::io_context io_context;
        boost::asio::signal_set signals(io_context,SIGINT,SIGTERM);
        signals.async_wait([this](const boost::system::error_code& error, int signal_number){
            if(!error){
            std::cout << "Shutdown signal received. Stopping server..." << std::endl;
            this->_server->Shutdown(); // Shutdown the server
            this->_cq->Shutdown();   // Shutdown the completion queue
            }
        });
        // 在单独的子线程中运行io_context
        std::thread([&io_context]() { io_context.run(); }).detach(); //分离子线程
        // Also run HandleRpcs in the main thread
        HandleRpcs();
    }
    // This can be run in multiple threads if needed.
void StatusServerImpl::HandleRpcs()  
    {
        //如果有多种请求 需要每种创建一个calldata进行注册到grpc框架接受请求返回事件到cq
        new GetChatServerCalldata(&_service,_cq.get(),this);
        void* tag;  // uniquely identifies a request.
        bool ok;
        while(true)
        {
             // Block waiting to read the next event from the completion queue. The
             // event is uniquely identified by its tag, which in this case is the
             // memory address of a CallData instance.
             if(!_cq->Next(&tag,&ok))
             {
                break; // Exit loop if Next() returns false (e.g., on shutdown)
             }
             if(!ok)
             {continue;}
             static_cast<GetChatServerCalldata*>(tag)->Proceed();
        }
    }
StatusServerImpl::~StatusServerImpl()
    {
        for(auto& th: _threads)
        {
            if(th.joinable())
            {th.join();}}
    }
