#pragma warning(disable:4996)

#include "Common.h"
#include <Shlwapi.h>

// 로그 파일 크기.
#define LOG_FILE_SIZE_MAX 31457280

//■=============================================================================================■
//   log type to string.
//■=============================================================================================■
const wchar_t* GetLogType(LOG_TYPE type)
{
	switch (type)
	{
	case LOG_TYPE_CRITICAL:   return L"CRITICAL";
	case LOG_TYPE_ERROR:      return L"ERROR";
	case LOG_TYPE_WARNING:    return L"WARNING";
	case LOG_TYPE_FAIL:       return L"FAIL";
	case LOG_TYPE_LAST_ERROR: return L"LAST_ERROR";
	}

	return L"INFO";
}

//■=============================================================================================■
//   log info.
//■=============================================================================================■
class LogInfo : public HNET::LIB::Memory
{
public:
	LogInfo(LOG_TYPE type)
        : _type(type)
        , _threadId(GetCurrentThreadId())
	{
		GetLocalTime(&_time);
		_text[0] = '\0';
	}

	LOG_TYPE   _type;
	DWORD      _threadId;
	SYSTEMTIME _time;
	wchar_t    _text[1024];
};

namespace HNET
{
    namespace LIB
    {
        Logger::Logger()
        {
            _Initialize(API::LogFilePath(), HNET::API::ExeFileName());

            // 메모리풀을 사용하므로 레퍼 증가.
            SINGLETON::MemoryReference();
        }

        Logger::Logger(const wchar_t* fileName)
        {
            _Initialize(API::LogFilePath(), fileName);

            // 메모리풀을 사용하므로 레퍼 증가.
            SINGLETON::MemoryReference();
        }

        Logger::Logger(const wchar_t* forder, const wchar_t* fileName)
        {
            _Initialize(forder, fileName);

            // 메모리풀을 사용하므로 레퍼 증가.
            SINGLETON::MemoryReference();
        }

        Logger::~Logger()
        {
			if (false == _active)
				return;

			_active = false;

			if (NULL != _thread)
			{
				WaitForSingleObject(_thread, INFINITE);
				_thread = NULL;
			}

            // 메모리풀을 사용했으므로 레퍼 감소.
            SINGLETON::MemoryRelease();
        }

		bool Logger::Write(LOG_TYPE type, const wchar_t* format, ...)
        {
			if (false == _active)
				return false;

			LogInfo* info = new LogInfo(type);
			if (NULL == info) return false;

			va_list Args;
			va_start(Args, format);
                API::StringVPrintf(info->_text, format, Args);
			va_end(Args);

			Lock();
			{
				_queue.push(info);
			}
			Unlock();

			return true;
        }

        void Logger::_Initialize(const wchar_t* forder, const wchar_t* fileName)
        {
            if (NULL == forder || NULL == fileName)
                return;

            // 초기화.
			_active		= true;
            _file       = NULL;
            _hour       = 0;
			_size		= 0;
            _number		= 0;

            // 파일 이름.
            API::StringCopy(_name, forder);
            API::StringCat(_name, L"/");
            API::StringCat(_name, fileName);

            // create forder.
            if (false == API::CreateFolder(forder))
            {
                MSG_BOX(L"error", L"[LOG] create folder failed=\"%s\"", _name);
                return;
            }

            // thread.
            unsigned int threadId;
            _thread = (HANDLE)_beginthreadex(NULL, 0, _Worker, this, 0, &threadId);
            if (0 == _thread)
            {
				_active = false;
                MSG_BOX(L"error", L"[LOG] thread failed=\"%s\"", _name);
                return;
            }
        }

		bool Logger::_OpenFile(SYSTEMTIME& time)
		{
			// 파일이 최대 크기를 초과, 또는 시간이 다르던가.
			if (NULL == _file || LOG_FILE_SIZE_MAX < _size || time.wHour != _hour)
			{
				wchar_t openFileName[1024] = L"";
                API::StringPrintf(openFileName, L"%s [%04d-%02d-%02d %02d'%02d'%02d]_%03d.log", _name, time.wYear, time.wMonth, time.wDay, time.wHour,time.wMinute, time.wSecond, _number++);

				// 파일 새로 생성.
				FILE* file = _wfopen(openFileName, L"wtc, ccs=UNICODE");
				if (NULL == file) return false;

				// 이전 파일 닫기.
				_CloseFile();

				// 새로 생성된 파일로 세팅.
				_file = file;

				// 새로운 일로 세팅.
				_hour = time.wHour;

				// 저장 크기 0으로 세팅.
				_size = 0;
			}

			return true;
		}

		void Logger::_CloseFile()
		{
			if (NULL == _file)
				return;

			fflush(_file);
			fclose(_file);
			_file = NULL;
		}

		void Logger::_Execute()
		{
			Lock();
			{
				while (false == _queue.empty())
				{
					LogInfo* info = _queue.front();
					_queue.pop();

					if (false == _OpenFile(info->_time))
						continue;

					wchar_t string[1024] = L"";
					API::StringPrintf(string, L"(%04d-%02d-%02d %02d:%02d:%02d.%03d)[%s]<%d> %s\n",
												info->_time.wYear,
												info->_time.wMonth,
												info->_time.wDay,
												info->_time.wHour,
												info->_time.wMinute,
												info->_time.wSecond,
												info->_time.wMilliseconds,
												GetLogType(info->_type),
												info->_threadId,
												info->_text);
					size_t size = wcslen(string) * sizeof(wchar_t);

					fwrite(string, size, 1, _file);

					_size += size;

					// output debug text.
					API::OutputDebugText(string);

					delete info;
				}
			}
			Unlock();

			fflush(_file);
		}

		unsigned WINAPI Logger::_Worker(void* param)
		{
			Logger* logger = (Logger*)param;

			while (logger->_active)
			{
				logger->_Execute();

				// 초당 5회 처리.
				HNET::API::Sleep(200);
			}

			return 0;
		}
    }
}