#include "Common.h"

void LogSpinLock(const wchar_t* funcName, int line)
{
    static thread_local ULONGLONG t_tick = 0;

    if (GetTickCount64() - t_tick < 3000)
        return;

    t_tick = GetTickCount64();

    if (line)
        LOG_CRITICAL_SYSTEM(L"function= %s, line= %d", funcName, line);
    else
        LOG_CRITICAL_SYSTEM(L"spin lock optimization is required");
}

namespace HNET
{
    namespace LIB
    {
        SpinLock::SpinLock()
            : _threadId(0)
            , _count(0)
        {

        }
        
        SpinLock::~SpinLock()
        {
            
        }

        void SpinLock::Lock(const wchar_t* funcName, int line)
        {
            DWORD curThreadId = GetCurrentThreadId();
            int   spinCount   = 0;
            
            while (InterlockedCompareExchange(&_count, 1, 0))
            {
                if (curThreadId == _threadId)
                {
                    ++_count;
                    return;
                }

                YieldProcessor(); // __asm pause

                if (4000 < ++spinCount)
                {
                    spinCount = 0;
                    LogSpinLock(funcName, line);
                }
            }

            _threadId = curThreadId;
        }

        void SpinLock::Unlock()
        {
            unsigned long lockCount = _count;
            if (--lockCount)
            {
                _count = lockCount;
                return;
            }

            _threadId  = 0;
            _count = 0;
        }
    }
}

ScopedSpinLock::ScopedSpinLock(HNET::LIB::SpinLock& lock, const wchar_t* funcName, int line)
    : _lock(lock)
{
    _lock.Lock(funcName, line);
}

ScopedSpinLock::~ScopedSpinLock()
{
    _lock.Unlock();
}