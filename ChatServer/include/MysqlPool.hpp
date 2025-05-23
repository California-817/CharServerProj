#pragma once
#include"Const.h"
#include<mysql.h>
//封装一个mysql连接带操作时间戳
class MYSQLCon
{
public:
    MYSQLCon(MYSQL* con,int64_t last_time):_last_operator_time(last_time),
    _con(con,[](MYSQL* p_con){ //连接销毁自动关闭
        mysql_close(p_con);})
    {}
    void UpdateTimeStamp() //更新时间戳
    {
        auto tmp=std::chrono::system_clock::now().time_since_epoch();
        int64_t now=std::chrono::duration_cast<std::chrono::seconds>(tmp).count(); //自1970到现在的秒时
        _last_operator_time=now;        
    }
public:
    std::shared_ptr<MYSQL> _con;
    int64_t _last_operator_time; //上次操作时间
};
class ConnectionPool
{
public:
    ConnectionPool(const std::string& ip,u_int16_t port,const std::string& user,
                             const std::string& password,const std::string& database,
                                size_t size,int max_wait_time)
    :_b_stop(false),_size(size),_max_wait_time(max_wait_time),
    _ip(ip),_port(port),_user(user),_password(password),_database(database)
    {
        auto tmp=std::chrono::system_clock::now().time_since_epoch();
        int64_t now=std::chrono::duration_cast<std::chrono::seconds>(tmp).count(); //自1970到现在的秒时
        for(int i=0;i<_size;i++)
        {
            MYSQL* con=mysql_init(nullptr);
            MYSQL* ret=mysql_real_connect(con,_ip.c_str(),_user.c_str(),_password.c_str(),_database.c_str(),_port,nullptr,0);
            if(ret==nullptr){ //连接失败
                std::cout<<"connection to mysql error"<<std::endl;
                mysql_close(con);
                continue;
            }
            auto my_con=std::make_shared<MYSQLCon>(con,now);
            _conns.push(std::move(my_con));
        }
         //启动检测连接线程
        _check_thread=std::thread([this](){
            int count=0;
            while(!_b_stop)
            {
                if(count>=MYSQL_CHECK_TIME){ //每隔60s进行连接的检测
                    CheckConnectionTask();
                    count=0;
                }
                count++;
                std::this_thread::sleep_for(std::chrono::seconds(1)); //每次休眠1s
            }
        }); //移动赋值
         _check_thread.detach();//分离线程 由os管理 出作用域即使正在运行 析构produce也不会出错
    }// 当执行到这里时，produce 和 scanner 会被析构如果线程对象析构时，线程仍在运行（即线程函数尚未完成），
    //C++标准库会调用   std::terminate 导致程序异常终止。这是因为线程对象的析构函数会检查线程是否仍在运行，如果仍在运行，则认为这是一个错误。

    //定时检测连接是否长时间未操作 否则进行操作发送心跳给mysql保持连接的活跃
    //定时回收因网络波动而无法使用的连接 并支持尝试重新连接因网络状况断开的连接
    void CheckConnectionTask() 
    {   
        int cur_count=0;
        //1.拿到当前的连接池中连接数
        {
            std::lock_guard<std::mutex> _lock(_mtx);
            cur_count=_conns.size();
        }
        auto now=std::chrono::system_clock::now().time_since_epoch();
        auto time_stamp=std::chrono::duration_cast<std::chrono::seconds>(now).count();
        //2.这个连接数不是绝对准确 但是获取连接进行处理时以此为基准
        int processed=0;
        while(processed<cur_count)
        {
            //2.1获取连接
            std::shared_ptr<MYSQLCon> mysql_con;
            {
                std::lock_guard<std::mutex> _lock(_mtx);
                if(_conns.empty()){
                    //连接已经没了 此时是因为cur_count要大于后来的size
                    break;
                }
                //此时后来的size大于等于cur_count 即使多的连接未遍历 下次定时遍历也会进行遍历
                mysql_con=_conns.front();
                _conns.pop();
            }
            bool IsHealthy=true; //标识这个连接是否异常
            //2.2判断是否长时间未操作
            if(time_stamp-mysql_con->_last_operator_time>MYSQL_EXPIRED_TIME)
            { //发送心跳包给mysql服务器
                if(mysql_query(mysql_con->_con.get(),"SELECT 1")!=0)
                {
                    //发送心跳包出错 进行异常连接回收处理
                    std::cout<<"connection to mysql error"<<std::endl;
                    _failed_count++;
                    IsHealthy=false;
                }
                mysql_con->UpdateTimeStamp(); //更新操作时间戳
            }
            if(IsHealthy)
            { //连接健康 放回连接池
                std::lock_guard<std::mutex> _lock(_mtx);
                _conns.push(std::move(mysql_con));
                _cv.notify_one();
            }
            //异常连接不放入连接池 智能指针引用计数==0 自动删除连接
        }
        //一次检测所有连接完毕 进行异常连接的重新建立连接
        while (_failed_count--)
        {
            MYSQL* con=mysql_init(nullptr);
            MYSQL* ret=mysql_real_connect(con,_ip.c_str(),_user.c_str(),_password.c_str(),_database.c_str(),_port,nullptr,0);
            if(ret==nullptr){ //重新连接失败
                std::cout<<"reconnection to mysql error"<<std::endl;
                mysql_close(con);
                break; //重连失败 后面的重连也就没必要在尝试 丢给下次定时检测来重连
            }
            std::cout<<"reconnect to mysql success "<<std::endl;
            {
                std::lock_guard<std::mutex> _lock(_mtx);
                auto my_con=std::make_shared<MYSQLCon>(con,time_stamp);
                _conns.push(std::move(my_con));   
                _cv.notify_one();  
            }      
        }
    }
    std::shared_ptr<MYSQLCon> GetConnection()
    {
        std::unique_lock<std::mutex> _ulock(_mtx);
        while(_conns.empty()&&!_b_stop)
        {  
            //等待一段时间
            std::cv_status ret=_cv.wait_for(_ulock,std::chrono::seconds(_max_wait_time));
            if(ret==std::cv_status::timeout)
            { //超时返回
                if(_conns.empty())
                { //无链接
                    std::cout<<"get connection timeout"<<std::endl;
                    return std::shared_ptr<MYSQLCon>(); //返回空指针
                }//超时但是有连接  
                }//未超时 有链接
        }
        if(_b_stop)
        { //暂停mysql连接池
            return std::shared_ptr<MYSQLCon>(); //返回空指针
        }//正常获取连接
        auto ret_con=_conns.front();
        _conns.pop();
        return ret_con;
    }
    void ReturnConnection(std::shared_ptr<MYSQLCon> return_con)
    {
        std::unique_lock<std::mutex> _ulock(_mtx);
        if(_b_stop){ //池子已经关闭 没必要还回队列
            return_con.reset();
            return;
        }
        //池子没关闭
        _conns.push(return_con);
        _cv.notify_one();
    }
    void Close()  //关闭连接池
    {
        _b_stop=true;
        _cv.notify_all();
    }
    ~ConnectionPool()
    {
        std::lock_guard<std::mutex> _lock(_mtx);
        Close();
        while(!_conns.empty()){
            _conns.pop();}
    }
private:
    std::queue<std::shared_ptr<MYSQLCon>> _conns;
    std::mutex _mtx; 
    std::condition_variable _cv; 
    std::atomic<bool> _b_stop; //是否暂停该连接池
    size_t  _size; //连接个数
    int _max_wait_time;  //消费者获取连接最长等待时间
    std::thread _check_thread; //检测连接线程
    std::string _ip; 
    uint16_t  _port;
    std::string _user;
    std::string _password;
    std::string _database;
    std::atomic<int> _failed_count; //表示一次遍历所有连接 获取到的失效的连接数
};

