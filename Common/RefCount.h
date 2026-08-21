#pragma once

#include <windows.h>

namespace HNET
{
    namespace LIB
    {
        class RefCount
        {
        public:
            RefCount(unsigned long count = 1);
            ~RefCount();
        
            void Reference();
            void Release();

            bool Invalid();

	    private:
		    virtual void OnRelease() {}

        private:
            unsigned long _count;
        };
    }
}