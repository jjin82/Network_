#pragma once

#include <windows.h>

namespace HNET
{
    namespace LIB
    {
        class Memory
        {
        public:
            void* operator new(size_t size);
            void operator delete(void* p);

            void* operator new[](size_t size);
            void operator delete[](void* p);
        };
    }

    namespace API
    {
        void* Alloc(size_t size);
        void Dealloc(void* p);

        template<typename T> void Constructor(T* p) { new(p) T; } // 생성자 호출 함수.
        template<typename T> void Destructor(T* p)  { p->~T(); }  // 소멸자 호출 함수.
    }

    namespace SINGLETON
    {
        void MemoryReference();
        void MemoryRelease();
    }
}