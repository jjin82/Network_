#pragma once

#include <windows.h>

namespace HNET
{
    namespace LIB
    {
        class Flag
        {
        public:
	        Flag(int count);

	        void Enable(const DWORD flag);
	        void Disable(const DWORD flag);
	        bool IsFlag(const DWORD flag);

        private:
            DWORD _maxCount;
	        bool* _flag;
        };
    }
}