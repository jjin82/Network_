#include "Common.h"

namespace HNET
{
    namespace LIB
    {
        RefCount::RefCount(unsigned long count)
            : _count(count)
        {

        }

        RefCount::~RefCount()
        {

        }

        void RefCount::Reference()
        {
            InterlockedIncrement(&_count);
        }

        void RefCount::Release()
        {
		    if (0 == InterlockedDecrement(&_count))
			    OnRelease();
        }

        bool RefCount::Invalid()
        {
            return (_count == 0);
        }
    }
}