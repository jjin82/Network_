#include "Common.h"

namespace HNET
{
    namespace LIB
    {
        Time::Time(time_t time)
        { 
            _time = API::ConvertToSystemTime(time); 
        }

	    Time::Time(SYSTEMTIME& time)
        { 
            _time = time; 
        }

	    Time::Time()
        { 
            GetLocalTime(&_time); 
        }

	    void Time::operator=(time_t time)
        { 
            _time = API::ConvertToSystemTime(time); 
        }

	    void Time::operator=(SYSTEMTIME& time) 
        { 
            _time = time; 
        }

	    Time::operator time_t()                   
        { 
            return API::ConvertToTime(_time); 
        }

	    Time::operator SYSTEMTIME()               
        { 
            return _time; 
        }
	
        Time::operator FILETIME()                 
        { 
            return API::ConvertToFileTime(_time); 
        }
    }
}