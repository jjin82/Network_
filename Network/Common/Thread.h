#pragma once

#include <windows.h>

namespace HNET
{
    namespace LIB
    {
        class Thread
        {
            virtual void OnWorker() = 0;

        protected:
	        Thread(const wchar_t* name = L"thread");

        public:
            virtual ~Thread();

	        bool Run();
		    void Wait(DWORD waitMillisecond = INFINITE); // 호출위치에서 signal 발생까지 대기.
            void Pulse();                                // signal 발생.

        private:
	        static unsigned WINAPI _Worker(void* param);

        private:
	        HANDLE  _thread;        // 핸들.
            HANDLE  _event;         // 이벤트.
            wchar_t _name[1024];    // 이름.
        };
    }
}