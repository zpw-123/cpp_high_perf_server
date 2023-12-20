#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include <memory>
namespace cpp_high_perf {

template <class T,  class X = void, int N = 0>
class Singleton {
public:
    static T* GetInstance() {
        static T v;
        return &v;
    }
};

//智能指针的单例模式
template <class T, class X = void, int N = 0>
class SingletonPtr {
public:
    static std::shared_ptr<T> GetInstance() {
        static std::shared_ptr<T> v(new T);
        return v;
    }
};

}

#endif