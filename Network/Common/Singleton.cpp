#include "Singleton.h"
#include "list"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSingletonManager
{
public:
    ~CSingletonManager()
    {
        for (auto var : _list)
            var->Release();
    }

    void Add(Singleton* singleton)
    {
        _list.push_back(singleton);
    }

private:
    std::list<Singleton*> _list;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Singleton::Singleton()
{
    static CSingletonManager s_singletonManager;
    s_singletonManager.Add(this);
}

Singleton::~Singleton()
{

}

void* Singleton::operator new(size_t size)
{
    return malloc(size);
}

void Singleton::operator delete(void* p)
{
    free(p);
}

void Singleton::OnRelease()
{
    this->~Singleton();

    free(this);
}