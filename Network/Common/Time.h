#pragma once

#include <windows.h>

namespace HNET
{
    namespace LIB
    {
        //бс=============================================================================================бс
        //   Time.
        //бс=============================================================================================бс
        class Time
        {
        public:
	        Time(time_t time);
	        Time(SYSTEMTIME& time);
	        Time();

        public:	
	        void operator=(time_t time);
	        void operator=(SYSTEMTIME& time);

        public:
	        operator time_t();
	        operator SYSTEMTIME();
	        operator FILETIME();

        private:
	        SYSTEMTIME _time;
        };
    }
}