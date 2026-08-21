#pragma once

#include <windows.h>

#define _W(X) L##X
#define __W(X) _W(X)

#define SCOPED_SLOCK(lock) ScopedSpinLock ScopedSpinLock(lock, __W(__FUNCTION__), __LINE__)

namespace HNET
{
    namespace LIB
    {
        class SpinLock
        {
        public:
            SpinLock();
            ~SpinLock();

	        void Lock(const wchar_t* funcName = L"", int line = 0);
	        void Unlock();

        private:
            unsigned long _count;
            unsigned long _threadId;
        };
    }
}

class ScopedSpinLock
{
public:
	ScopedSpinLock(HNET::LIB::SpinLock& lock, const wchar_t* funcName, int line);
	~ScopedSpinLock();

private:
	HNET::LIB::SpinLock& _lock;
};