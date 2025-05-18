#include"../include/DistLock.h"
//加分布式锁
std::string DistLock::acquireLock(const std::string& lock_key,int locktimeout,int acquiretime)
{
    std::string key=LOCK_PREFIX;
    key+=lock_key; //lock:uid
    // 创建一个uuid唯一标识加锁者
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    std::string uuid = boost::uuids::to_string(a_uuid);
    auto redis_con=RedisMgr::GetInstance()->GetRedisCon();
    auto now=std::chrono::system_clock::now();
    auto end_time=now+std::chrono::seconds(acquiretime); //获取锁的最后超时时间
    // //构造命令nx与ex
    // sw::redis::CmdArgs cmd;
    // cmd.append("SET").append(key).append(uuid).append("NX").append("EX").append(std::to_string(locktimeout));
    // // 2. 获取参数数组和长度
    // const auto& argv = cmd.argv();     // const char** 参数数组
    // auto len = cmd.argv_len();        // 参数个数
    while(std::chrono::system_clock::now()<end_time)
    { //没有超时
        auto result=redis_con->command("SET",key,uuid,"NX","EX",std::to_string(locktimeout));
        if(result&&result->type==REDIS_REPLY_STATUS&&std::string(result->str)=="OK")
        { //加锁成功
            std::cout<<"加锁成功"<<key<<std::endl;
            return uuid;
        }
        //加锁失败
        std::this_thread::sleep_for(std::chrono::milliseconds(10));//休眠10ms 再次尝试加锁
    }
    //加锁超时
    return "";
}
//解锁分布式锁
bool DistLock::releaseLock(const std::string& lock_key,const std::string& uuid)
{
    std::string key=LOCK_PREFIX;
    key+=lock_key; //lock:uid   
    //构建一个lua脚本进行解锁
    std::string luaScript="if redis.call('GET',KEYS[1]) == ARGV[1] then \
                                return redis.call('DEL',KEYS[1]) \
                           else \
                                return 0 \
                           end";
    auto redis_con=RedisMgr::GetInstance()->GetRedisCon();
    auto shaReply=redis_con->script_load(luaScript); //预编译lua脚本 不执行 返回脚本的摘要
    // sw::redis::CmdArgs cmd;
    // cmd.append("EVALSHA").append(shaReply).append("1").append(key).append(uuid);
    // // 2. 获取参数数组和长度
    // const auto& argv = cmd.argv();     // const char** 参数数组
    // auto len = cmd.argv_len();        // 参数个数
    auto result=redis_con->command("EVALSHA",shaReply,"1",key,uuid);
    if(result&&result->type==REDIS_REPLY_INTEGER&&result->integer==1)
    {//解锁成功
        std::cout<<"解锁成功"<<std::endl;
        return true;
    }
    return false;

}