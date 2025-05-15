#include "../include/ChatGrpcServerImpl.h"
ChatServerImpl::ChatServerImpl()
{}
void ChatServerImpl::Run(uint16_t port)
{
    std::string server_address = "0.0.0.0:";
    server_address += std::to_string(port);
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&_service);
    _cq = builder.AddCompletionQueue();
    _server = builder.BuildAndStart();
    std::cout << "Server listening on " << server_address << std::endl;
    for (int i = 0; i < std::thread::hardware_concurrency(); i++)
    { //多个子线程等待grpc请求并进行逻辑处理
        _threads.emplace_back(&ChatServerImpl::HandleRpcs, this);
    }
    // boost::asio::io_context io_context;
    // boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    // signals.async_wait([this](const boost::system::error_code &error, int signal_number)
    //                    {
    //         if(!error){
    //         std::cout << "Shutdown signal received. Stopping server..." << std::endl;
    //         this->_server->Shutdown(); // Shutdown the server
    //         this->_cq->Shutdown();   // Shutdown the completion queue
    //         } });
    // 在单独的子线程中运行io_context
    // std::thread([&io_context]()
    //             { io_context.run(); })
    //     .detach(); // 分离子线程
    //主线程执行到这直接退出
}
// This can be run in multiple threads if needed.
void ChatServerImpl::HandleRpcs()
{
    // 如果有多种请求 需要每种创建一个calldata进行注册到grpc框架接受请求返回事件到cq
    new NotifyAddFriendCalldata(&_service, _cq.get(), this); //添加好友的请求放入完成队列
    void *tag; // uniquely identifies a request.
    bool ok;
    while (true)
    {
        // Block waiting to read the next event from the completion queue. The
        // event is uniquely identified by its tag, which in this case is the
        // memory address of a CallData instance.
        // Next方法：这是一个阻塞方法，用于从队列中获取完成的事件。它可以在多个线程中并发调用，而不会导致数据竞争
        if (!_cq->Next(&tag, &ok))
        {
            break; // Exit loop if Next() returns false (e.g., on shutdown)
        }
        if (!ok)
        {
            continue;
        }
        static_cast<Calldata *>(tag)->Proceed(); // 多态进行处理
    }
}
ChatServerImpl::~ChatServerImpl()
{
    std::cout << "Shutdown signal received. Stopping grpc server..." << std::endl;
    for (auto &th : _threads)
    {
        if (th.joinable())
        { //主线程等待子线程退出
            th.join();
        } 
    }
}