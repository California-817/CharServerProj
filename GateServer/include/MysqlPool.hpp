#pragma once
#include"Const.h"
#include<mysql.h>
class ConnectionPool
{
public:
    ConnectionPool(const std::string& ip,u_int16_t port,const std::string& user,
                             const std::string& password,const std::string& database,
                                size_t size,int max_wait_time)
    :_b_stop(false),_size(size),_max_wait_time(max_wait_time),
    _ip(ip),_port(port),_user(user),_password(password),_database(database)
    {
        for(int i=0;i<_size;i++)
        {
            MYSQL* con=mysql_init(nullptr);
            MYSQL* ret=mysql_real_connect(con,_ip.c_str(),_user.c_str(),_password.c_str(),_database.c_str(),_port,nullptr,0);
            if(ret==nullptr)
            {std::cout<<"connection to mysql error"<<std::endl;}
             _conns.push(con);
        }
        // //启动生产连接线程
        // std::thread produce(std::bind(&ProduceConnectionTask,this));
        // produce.detach();//分离线程 由os管理 出作用域即使正在运行 析构produce也不会出错
        // //启动监管线程
        // std::thread scanner(std::bind(&ScannerConnectionTask,this));
        // scanner.detach();
    }// 当执行到这里时，produce 和 scanner 会被析构如果线程对象析构时，线程仍在运行（即线程函数尚未完成），
    //C++标准库会调用   std::terminate 导致程序异常终止。这是因为线程对象的析构函数会检查线程是否仍在运行，如果仍在运行，则认为这是一个错误。
    std::shared_ptr<MYSQL> GetConnecion()
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
                    return std::shared_ptr<MYSQL>(); //返回空指针
                }
                //超时但是有连接
            }
            //未超时 有链接
        }
        if(_b_stop)
        { //暂停mysql连接池
            return std::shared_ptr<MYSQL>(); //返回空指针
        }
        MYSQL* con=_conns.front();
        _conns.pop();
        std::shared_ptr<MYSQL> ret_con(con,[this](MYSQL* con){
            std::lock_guard<std::mutex> _lock(_mtx);
            if(_b_stop)
            {  //连接池已经关闭 没必要再将连接放入了 直接close
                mysql_close(con);
                return;
            }
            _conns.push(con);
            _cv.notify_one();
        });
        return ret_con;
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
        while(!_conns.empty())
        {
            mysql_close(_conns.front());
            _conns.pop();
        }
    }
private:
    std::queue<MYSQL*> _conns;
    std::mutex _mtx; 
    std::condition_variable _cv; 
    std::atomic<bool> _b_stop; //是否暂停该连接池
    size_t  _size; //初始连接
    int _max_wait_time;  //消费者获取连接最长等待时间
    std::string _ip; 
    uint16_t  _port;
    std::string _user;
    std::string _password;
    std::string _database;
};

