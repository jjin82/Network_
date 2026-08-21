#pragma once

#include <windows.h>
#include "RefCount.h"

class Singleton : public HNET::LIB::RefCount
{
public:
    Singleton();
    virtual ~Singleton();

    void* operator new(size_t size);
    void operator delete(void* p);

private:
    void OnRelease() override;
};

template<typename T>
class SingletonT : public Singleton, public T
{
public:
    SingletonT() {}
    virtual ~SingletonT() {}
};

namespace HNET
{
    namespace SINGLETON
    {
        template<typename T>
        SingletonT<T>* Singleton()
        {
            static SingletonT<T> p;
            return &p;
        }
    }
}