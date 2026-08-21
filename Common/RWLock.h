#pragma once

#include <windows.h>

#define SCOPED_WLOCK(lock) ScopedRWLock ScopedRWLock(lock, true)
#define SCOPED_RLOCK(lock) ScopedRWLock ScopedRWLock(lock, false)

namespace HNET
{
    namespace LIB
    {
        class RWLock
        {
        public:
            RWLock();
            ~RWLock();

            void LockW();
            void UnlockW();

            void LockR();
            void UnlockR();

        private:
            SRWLOCK	_lock;
        };
    }
}

class ScopedRWLock
{
public:
	ScopedRWLock(HNET::LIB::RWLock& lock, bool write);
	~ScopedRWLock();

private:
	bool                _write;
	HNET::LIB::RWLock& _lock;
};

