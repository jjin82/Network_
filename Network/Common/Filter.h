#pragma once

#include <windows.h>
#include <atlstr.h>  // string.
#include <atlcoll.h> // map

namespace HNET
{
    namespace LIB
    {
        class Filter : public CAtlMap<wchar_t, Filter*>
        {
        public:
	        Filter();
	        ~Filter();

	        bool Register(const wchar_t* string);
	        bool IsExist(const wchar_t* string, OUT wchar_t* filter, size_t lenth = 0);
        };
    }
}