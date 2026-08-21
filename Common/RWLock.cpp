#include "Common.h"

namespace HNET
{
    namespace LIB
    {
	    RWLock::RWLock()
	    {
		    InitializeSRWLock(&_lock);
	    }
	    
        RWLock::~RWLock()
	    {
	
	    }

        void RWLock::LockW()
        {
            AcquireSRWLockExclusive(&_lock);
        }

        void RWLock::UnlockW()
        {
            ReleaseSRWLockExclusive(&_lock);
        }

        void RWLock::LockR()
        {
            AcquireSRWLockShared(&_lock);
        }

        void RWLock::UnlockR()
        {
            ReleaseSRWLockShared(&_lock);
        }
    }
}

ScopedRWLock::ScopedRWLock(HNET::LIB::RWLock& lock, bool write)
    : _lock(lock)
    , _write(write)
{
    if(_write)
	    _lock.LockW();
    else
        _lock.LockR();
}

ScopedRWLock::~ScopedRWLock()
{
    if (_write)
        _lock.UnlockW();
    else
        _lock.UnlockR();
}