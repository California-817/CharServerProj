#pragma once
#include"Const.h"
// 基类模板单例对象
template <class T>
class Singleton
{
public:
    static std::shared_ptr<T>& GetInstance()
    {
        //static成员只在第一次调用的时候初始化
        static std::once_flag s_flag;
        //这个函数只会调用一次 多线程安全
        std::call_once(s_flag,[&](){
            _instance=std::make_shared<T>();
        });
        return _instance;
    }
protected:
    Singleton()
    {}
    Singleton(const Singleton<T>&)=delete;
    Singleton<T>& operator=(const Singleton<T>&)=delete;
protected:
    static std::shared_ptr<T> _instance;
};
template<class T>
std::shared_ptr<T> Singleton<T>::_instance(nullptr);