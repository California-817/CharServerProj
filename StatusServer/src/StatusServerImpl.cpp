#include"../include/StatusServerImpl.h"
StatusServerImpl::StatusServerImpl()
    {
        mINI::INIFile file("../conf/config.ini");
        mINI::INIStructure ini;
        file.read(ini);
        std::string section="ChatServers";
        std::string key="servers";
        std::string value=ini[section][key];
        // 分割字段值
        std::vector<std::string> values;
        std::stringstream ss(value);
        std::string item;
         while (std::getline(ss, item, ',')) {
            values.push_back(item);}
        for(auto& e:values)
        {
            _chat_servers.insert(std::make_pair(ini[e]["name"],ChatServer_struct(ini[e]["host"],ini[e]["port"],ini[e]["name"],0)));
        }
    }
void StatusServerImpl::Run(uint16_t port)
    {
        std::string server_address="0.0.0.0:";
        server_address+=std::to_string(port);
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
        new LoginCalldata(&_service,_cq.get(),this);
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
             static_cast<Calldata*>(tag)->Proceed();//多态
        }
    }
StatusServerImpl::~StatusServerImpl()
    {
        for(auto& th: _threads)
        {
            if(th.joinable())
            {th.join();}}
    }
