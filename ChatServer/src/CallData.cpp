#include "../include/CallData.h"
#include "../include/ChatGrpcServerImpl.h"
#include "../include/Session.h"
Calldata::Calldata(ChatService::AsyncService *service, ServerCompletionQueue *cq, ChatServerImpl *impl)
    : _service(service), _cq(cq), _status(CREATE), _impl(impl)
{
}

NotifyAddFriendCalldata::NotifyAddFriendCalldata(ChatService::AsyncService *service, ServerCompletionQueue *cq, ChatServerImpl *impl)
    : Calldata(service, cq, impl), _responder(&_ctx)
{
    // Invoke the serving logic right away. 立即调用服务逻辑
    Proceed();
}
void NotifyAddFriendCalldata::Proceed()
{
    if (_status == CREATE)
    {
        // Make this instance progress to the PROCESS state.
        _status = PROCESS;
        // 将这个calldata实例注册到异步grpc框架用来保存解析到的一个请求并放入完成队列
        // this指针作为这次注册的tag标记,用于从完成队列获取事件时找到该calldata实例
        _service->RequestNotifyAddFriend(&_ctx, &_req, &_responder, _cq, _cq, this);
    }
    else if (_status == PROCESS)
    {
        // 已经获取到了一个请求,现在进行处理请求并生成响应放入grpc框架的完成队列
        // Spawn new instance for next request 重新生成一个Calldata用于下次接收请求
        // 要形成一个calldata链,每次一个calldata使用后 需要自动生成一个新的calldata继续接收请求
        new NotifyAddFriendCalldata(_service, _cq, _impl);
        // 真正的进行请求处理并生成响应
        std::string prefix("my chat server has received : addfriend request");
        std::cout << prefix << std::endl;
        // 添加好友处理逻辑
        // 找到对方的session
        auto to_session = UserMgr::GetInstance()->GetUidSession(_req.touid());
        if (to_session.get())
        { // 对方在线
            nlohmann::json to_root;
            to_root["error"] = ErrorCodes::Success;
            to_root["applyuid"] = _req.applyuid();
            to_root["name"] = _req.name();
            to_root["desc"] = _req.desc();
            to_root["icon"] = _req.icon();
            to_root["nick"] = _req.nick();
            to_root["sex"] = _req.sex();
            std::string to_jsonstr = to_root.dump(4);
            // 给对方发送申请好友请求
            to_session->Write(MSGID_NOTIFY_ADD_FRIEND, to_jsonstr.size(), to_jsonstr.c_str());
        }
        _rsp.set_error(ErrorCodes::Success);
        _rsp.set_applyuid(_req.applyuid());
        _rsp.set_touid(_req.touid());
        // 处理完毕
        _status = FINISH;
        // 告诉grpc框架处理完毕发送响应 进行一次注册逻辑 当grpc框架处理完毕后通过完成队列返回一个事件
        // 此时通过tag获取这个calldata对象进行销毁处理
        _responder.Finish(_rsp, Status::OK, this);
    }
    else
    {
        // Once in the FINISH state, deallocate ourselves (CallData).
        delete this; // 删除这个calldata对象
    }
}

NotifyAuthFriendCalldata::NotifyAuthFriendCalldata(ChatService::AsyncService *service, ServerCompletionQueue *cq, ChatServerImpl *impl)
    : Calldata(service, cq, impl), _responder(&_ctx)
{
    // Invoke the serving logic right away. 立即调用服务逻辑
    Proceed();
}
void NotifyAuthFriendCalldata::Proceed()
{
    if (_status == CREATE)
    {
        // Make this instance progress to the PROCESS state.
        _status = PROCESS;
        // 将这个calldata实例注册到异步grpc框架用来保存解析到的一个请求并放入完成队列
        // this指针作为这次注册的tag标记,用于从完成队列获取事件时找到该calldata实例
        _service->RequestNotifyAuthFriend(&_ctx, &_req, &_responder, _cq, _cq, this);
    }
    else if (_status == PROCESS)
    {
        // 已经获取到了一个请求,现在进行处理请求并生成响应放入grpc框架的完成队列
        // Spawn new instance for next request 重新生成一个Calldata用于下次接收请求
        // 要形成一个calldata链,每次一个calldata使用后 需要自动生成一个新的calldata继续接收请求
        new NotifyAuthFriendCalldata(_service, _cq, _impl);
        // 真正的进行请求处理并生成响应
        std::string prefix("my chat server has received : authfriend request");
        std::cout << prefix << std::endl;
        // 通知认证好友处理逻辑
        // 找到对方的session
        auto to_session = UserMgr::GetInstance()->GetUidSession(_req.touid());
        if (to_session.get())
        { // 对方在线 将自己的信息返回对方
            UserInfo userinfo;
            _impl->GetUserInfo(_req.fromuid(), userinfo);
            // int err = jsonObj["error"].toInt();
            // int from_uid = jsonObj["fromuid"].toInt();
            // QString name = jsonObj["name"].toString();
            // QString nick = jsonObj["nick"].toString();
            // QString icon = jsonObj["icon"].toString();
            // int sex = jsonObj["sex"].toInt();
            nlohmann::json to_root;
            to_root["error"] = ErrorCodes::Success;
            to_root["fromuid"] = userinfo._uid;
            to_root["name"] = userinfo._name;
            to_root["desc"] =userinfo._desc;
            to_root["icon"] =userinfo._icon;
            to_root["nick"] =userinfo._nick;
            to_root["sex"] =userinfo._sex;
            std::string to_jsonstr = to_root.dump(4);
            // 给对方发送认证好友请求
            to_session->Write(MSGID_NOTIFY_AUTH_FRIEND, to_jsonstr.size(), to_jsonstr.c_str());
        }
        _rsp.set_error(ErrorCodes::Success);
        _rsp.set_fromuid(_req.fromuid());
        _rsp.set_touid(_req.touid());
        // 处理完毕
        _status = FINISH;
        // 告诉grpc框架处理完毕发送响应 进行一次注册逻辑 当grpc框架处理完毕后通过完成队列返回一个事件
        // 此时通过tag获取这个calldata对象进行销毁处理
        _responder.Finish(_rsp, Status::OK, this);
    }
    else
    {
        // Once in the FINISH state, deallocate ourselves (CallData).
        delete this; // 删除这个calldata对象
    }
}
