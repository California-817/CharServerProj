#pragma once
#include "Session.h"
#include "LogicSystem.h"
#include "Const.h"
// 基类消息节点
class MsgNode
{
public:
    MsgNode(uint32_t total_len);
    void Clear();
    ~MsgNode();

public:
    uint32_t _cur_len;
    uint32_t _total_len;
    char *_data;
};
// 接受消息节点
class RecvNode : public MsgNode
{
public:
    RecvNode(uint16_t &msg_id, uint16_t &len);
    uint16_t _msg_id;
};
// 发送消息节点 消息需要被组织成tlv格式 type length value [2 2 size]
class SendNode : public MsgNode
{
public:
    SendNode(uint16_t &msg_id, uint16_t &len,const char *&data);
    uint16_t _msg_id;
};
class LogicNode //逻辑节点
{
public:
    friend class LogicSystem; // 逻辑单元可以访问这个消息节点
    LogicNode(std::shared_ptr<Session> session, std::shared_ptr<RecvNode> recvnode);
    std::shared_ptr<Session> GetSession();
    std::shared_ptr<RecvNode> GetRecvNode();
private:
    std::shared_ptr<Session> _session; //这个消息节点所属的session
    std::shared_ptr<RecvNode> _recvnode; //真正的消息节点
};