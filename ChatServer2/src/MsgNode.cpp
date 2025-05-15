#include "../include/MsgNode.h"
MsgNode::MsgNode(uint32_t total_len)
    : _cur_len(0), _total_len(total_len), _data(new char[_total_len + 1]())
{
    memset(_data, 0, _total_len);
    _data[_total_len] = '\0';
}
void MsgNode::Clear()
{
    memset(_data,0,_total_len);
}
MsgNode::~MsgNode()
{
    // 释放节点的堆上空间
    delete[] _data;
}
// 接受消息节点
RecvNode::RecvNode(uint16_t &msg_id, uint16_t &len)
    : MsgNode(len), _msg_id(msg_id)
{
}
// 发送消息节点 消息需要被组织成tlv格式 type length value [2 2 size]
SendNode::SendNode(uint16_t &msg_id, uint16_t &len, const char *&data)
    : MsgNode(len + HEAD_TOTAL_LEN)
{
    // 1.id和length转网络字节序并拷入
    uint16_t id_ = htons(msg_id);
    uint16_t len_ = htons(len);
    memcpy(_data, &id_, HEAD_ID_LEN);
    memcpy(_data + HEAD_ID_LEN, &len_, HEAD_DATA_LEN);
    // 2.拷贝逻辑层生成的数据
    memcpy(_data + HEAD_TOTAL_LEN, data, len);
}
// 逻辑节点
LogicNode::LogicNode(std::shared_ptr<Session> session, std::shared_ptr<RecvNode> recvnode)
    : _session(session), _recvnode(recvnode) {}
std::shared_ptr<Session> LogicNode::GetSession()
{
    return _session;
}
std::shared_ptr<RecvNode> LogicNode::GetRecvNode()
{
    return _recvnode;
}
