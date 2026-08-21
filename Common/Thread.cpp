#include "Common.h"

namespace HNET
{
    namespace LIB
    {
        Thread::Thread(const wchar_t* name)
            : _thread(NULL)
            , _event(NULL)
        {
			API::StringCopy(_name, name);

            // 로그를 사용하므로 레퍼 증가.
            SINGLETON::SystemLogReference();
        }

        Thread::~Thread()
        {
            HANDLE closeEvent = _event;
            _event = NULL;

            SetEvent(closeEvent);

            // 쓰레드 종료 시점까지 대기.
            WaitForSingleObject(_thread, INFINITE);

            CloseHandle(closeEvent);
            CloseHandle(_thread);

            // 로그를 사용하므로 레퍼 감소.
            SINGLETON::SystemLogRelease();
        }

        bool Thread::Run()
        {
            if (_thread)
                return true;

            _event = ::CreateEvent(NULL, FALSE, FALSE, NULL); // 2번 인자가 FALSE 여서 자동 리셋.(자동 리셋은 하나의 쓰레드의 Wait 상태를 풀어줌)
            if (NULL == _event)
            {
                LOG_CRITICAL_SYSTEM(L"not use event in \"%s\" thread.", _name);
                return false;
            }

            unsigned int threadId;
	        _thread = (HANDLE)_beginthreadex(NULL, 0, _Worker, this, CREATE_SUSPENDED, &threadId);
	        if (0 == _thread)
            {
                LOG_CRITICAL_SYSTEM(L"begin thread=\"%s\".", _name);
                CloseHandle(_event);
		        return false;
            }

	        SetThreadPriority(_thread, THREAD_PRIORITY_NORMAL);
	        ResumeThread(_thread);

	        return true;
        }

        void Thread::Wait(DWORD waitMillisecond)
        {
	        WaitForSingleObject(_event, waitMillisecond);
        }

        void Thread::Pulse()
        {
            SetEvent(_event); // 자동 리셋 event 이어서 ResetEvent()를 안해도 됨.
        }

	    unsigned WINAPI Thread::_Worker(void* param)
        {
	        Thread* thread = (Thread*)param;

            LOG_INFO_SYSTEM(L"begin thread. id=\"%d\", name=\"%s\"", GetCurrentThreadId(), thread->_name);

            while (thread->_event)
            {
                thread->OnWorker();
            }

            LOG_INFO_SYSTEM(L"end thread. id=\"%d\", name=\"%s\"", GetCurrentThreadId(), thread->_name);

	        return 0;
        }
    }
}