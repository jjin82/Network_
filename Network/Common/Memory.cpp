#pragma warning(disable:4100)

#include "Common.h"
#include <list>

//■=============================================================================================■
//   메모리 노드.
//■=============================================================================================■
class MemoryNode
{
    friend class CMemoryStack;
    friend class MemoryPool;

private:
    USHORT       _size;
    DWORD        _threadId;
    MemoryNode* _next;
};

//■=============================================================================================■
//   메모리 스택.
//■=============================================================================================■
class CMemoryStack
{
public:
    void Push(MemoryNode* node)
    {
        node->_threadId = 0;
        node->_next     = _head;
        _head           = node;
    }

    bool Pop(OUT MemoryNode*& node)
    {
        if (NULL == _head)
            return false;

        node  = _head;
        _head = node->_next;

        return true;
    }

public:
    MemoryNode* _head;
};

//■=============================================================================================■
//   메모리 풀.
//■=============================================================================================■
class MemoryPool
{
    typedef std::list<char*> Buffers;

    enum
    { 
        MAX_SIZE   = 0xffff,           // MAX_SIZE를 넘기기 않는다.
        ALLOC_SIZE = MAX_SIZE * 20,    // MAX_SIZE를 n개 할당.
        FILL_COUNT = 32                // tls stack 채워지는 개수. 
    };

public:
    MemoryPool()
    {
        _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
        
        ZeroMemory(_chunk, sizeof(_chunk));
    }

    virtual ~MemoryPool()
    {
        for (auto& buf : _buffers)
            delete buf;

        _buffers.clear();
    }

    CMemoryStack* GetStack(size_t size)
    {
        if (MAX_SIZE <= size)
        {
            MSG_BOX(L"error", L"memory pool size exceeded= %I64d / %d", size, MAX_SIZE);
            return NULL;
        }

        static thread_local CMemoryStack t_stack[MAX_SIZE] = { 0, };
        return &t_stack[size];
    }

    MemoryNode* Allocate(size_t size)
    {
        CMemoryStack* stack = GetStack(size);
        if (NULL == stack) return NULL;

        MemoryNode* node = NULL;
        while (false == stack->Pop(node))
        {
            if (false == Fill(stack, size))
                return NULL;
        }

        node->_threadId = GetCurrentThreadId();

        return node;
    }

    void Deallocate(MemoryNode* node)
    {
        if (0 == node->_threadId)
            return;

        CMemoryStack* stack = GetStack(node->_size);
        if (NULL == stack) return;

        if (node->_threadId == GetCurrentThreadId())
        {
            stack->Push(node);
        }
        else
        {
            _lock.LockW();
            _chunk[node->_size].Push(node);
            _lock.UnlockW();
        }
    }

    bool Fill(OUT CMemoryStack* stack, size_t size)
    {
        MemoryNode* node  = NULL;
        int         count = 0;

        _lock.LockW();

        while (FILL_COUNT > count++)
        {
            if (_chunk[size].Pop(node))
            {
                stack->Push(node);
            }
            else
            {
                static thread_local size_t t_pop = ALLOC_SIZE;
                static thread_local char*  t_buf = NULL;

                if ((size + sizeof(MemoryNode)) > (ALLOC_SIZE - t_pop))
                {
                    t_buf = (char*)malloc(sizeof(char) * ALLOC_SIZE);
                    if (NULL == t_buf)
                    {
                        _lock.UnlockW();
                        return false;
                    }

                    _buffers.push_back(t_buf);
                    
                    t_pop = 0;
                }

                node         = (MemoryNode*)&t_buf[t_pop];
                node->_size  = (USHORT)size;
                t_pop       += (size + sizeof(MemoryNode));
                stack->Push(node);
            }
        }

        _lock.UnlockW();

        return true;
    }

private:
    HNET::LIB::RWLock _lock;
    Buffers           _buffers;         // 할당한 버퍼.
    CMemoryStack      _chunk[MAX_SIZE]; // 할당, 해제 쓰레드가 다른 경우 담는다.
};

namespace HNET
{
    namespace LIB
    {
        void* Memory::operator new(size_t size)
        {
            static MemoryPool* pool = HNET::SINGLETON::Singleton<MemoryPool>(); // performance
            return (pool->Allocate(size) + 1);
        }

        void Memory::operator delete(void* p)
        {
            static MemoryPool* pool = HNET::SINGLETON::Singleton<MemoryPool>();; // performance
            pool->Deallocate((MemoryNode*)p - 1);
        }

        void* Memory::operator new[](size_t size)
        {
            static MemoryPool* pool = HNET::SINGLETON::Singleton<MemoryPool>(); // performance
            return (pool->Allocate(size) + 1);
        }

        void Memory::operator delete[](void* p)
        {
            static MemoryPool* pool = HNET::SINGLETON::Singleton<MemoryPool>(); // performance
            pool->Deallocate((MemoryNode*)p - 1);
        }
    }

    namespace API
    {
        void* Alloc(size_t size)
        {
            static MemoryPool* pool = HNET::SINGLETON::Singleton<MemoryPool>(); // performance
            return (pool->Allocate(size) + 1);
        }

        void Dealloc(void* p)
        {
            static MemoryPool* pool = HNET::SINGLETON::Singleton<MemoryPool>(); // performance
            if (p) pool->Deallocate((MemoryNode*)p - 1);
        }
    }

    namespace SINGLETON
    {
        void MemoryReference()
        {
            HNET::SINGLETON::Singleton<MemoryPool>()->Reference();
        }

        void MemoryRelease()
        {
            HNET::SINGLETON::Singleton<MemoryPool>()->Release();
        }
    }
}