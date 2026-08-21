#pragma once
#pragma warning(disable: 4995 4481)

#include "Singleton.h"
#include "SpinLock.h"
#include <queue>

enum LOG_TYPE
{ 
	LOG_TYPE_NONE,
	LOG_TYPE_INFO,
	LOG_TYPE_CRITICAL,
	LOG_TYPE_ERROR,
	LOG_TYPE_WARNING,
	LOG_TYPE_FAIL,
    LOG_TYPE_LAST_ERROR,
};

class LogInfo;

namespace HNET
{
    namespace LIB
    {
        class Logger : public SpinLock
        {
        public:
            Logger();
	        Logger(const wchar_t* fileName);
            Logger(const wchar_t* forder, const wchar_t* fileName);
	        ~Logger();

            bool Write(LOG_TYPE type, const wchar_t* format, ...);

        private:
            void _Initialize(const wchar_t* forder, const wchar_t* fileName);
			bool _OpenFile(SYSTEMTIME& time);
			void _CloseFile();

			void _Execute();
	        
		private:
			static unsigned WINAPI _Worker(void* param);

        private:
			bool    _active;
	        FILE*   _file;
			HANDLE  _thread;           // thread.
            wchar_t _name[1024];       // 파일 이름.
	        size_t  _size;             // 파일 크기.
	        int     _number;           // 파일 넘버링.
			WORD    _hour;             // 일별 파일 갱신.

			std::queue<LogInfo*> _queue;
        };
    }
}