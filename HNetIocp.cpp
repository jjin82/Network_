#include "HNetSession.h"
#include "HNetIocp.h"


unsigned WINAPI Worker(void* arg);
unsigned WINAPI WorkerEx(void* arg);
unsigned WINAPI WorkerJob(void* arg);

HNetIocp::HNetIocp()
    : _iocp(NULL)
    , _thread(NULL)
    , _threadCount(0)
{
    // 로그를 사용하므로 레퍼 증가.
    HNET::SINGLETON::SystemLogReference();
}

HNetIocp::~HNetIocp()
{
    _Destroy();

    // 로그를 사용하므로 레퍼 감소.
    HNET::SINGLETON::SystemLogRelease();
}

bool HNetIocp::_Create(USHORT threadCount, bool single)
{
    _iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, threadCount);
    if (NULL == _iocp)
    {
        LOG_LAST_ERROR_DETAIL_SYSTEM();
        return false;
    }

    _threadCount = threadCount;
    _thread = new(std::nothrow) HANDLE[_threadCount];
    if (NULL == _thread)
        return false;

    LOG_INFO_SYSTEM(L"◆ message handling= \"%s\"", single ? L"single thread" : L"multi thread");

    for (USHORT i = 0; i < _threadCount; ++i)
    {
        unsigned int threadId;
        _thread[i] = (HANDLE)_beginthreadex(NULL, 0, (single ? WorkerJob : Worker), &_iocp, 0, &threadId);
        if (0 == _thread[i])
        {
            LOG_LAST_ERROR_DETAIL_SYSTEM();
            return false;
        }
    }

    return true;
}

void HNetIocp::_Destroy()
{
    if (NULL == _iocp)
        return;

	for (USHORT i = 0; i < _threadCount; ++i)
		PostQueuedCompletionStatus(_iocp, 0, 0, NULL);
	
	WaitForMultipleObjects(_threadCount, _thread, TRUE, INFINITE);

	CloseHandle(_iocp);
	_iocp = NULL;

    delete[] _thread;
}

bool HNetIocp::_Associate(HNetSession* session)
{
    if (NULL == CreateIoCompletionPort(*session, _iocp, 0, 0))
    {
        LOG_LAST_ERROR_DETAIL_SYSTEM();
        return false;
    }

    return true;
}

bool HNetIocp::_PostIo(HNetIo* io)
{
    return (0 != PostQueuedCompletionStatus(_iocp, 0, 0, io));
}

unsigned WINAPI Worker(void* arg)
{
    HANDLE         iocp       = *(HANDLE*)arg;
    DWORD          bytes      = 0;
    ULONG_PTR      key        = 0;
    OVERLAPPED*    overlapped = NULL;
    const wchar_t* name       = L"network io";

    LOG_INFO_SYSTEM(L"begin thread. id=\"%d\", name=\"%s\"", GetCurrentThreadId(), name);

    while (true)
    {
        GetQueuedCompletionStatus(iocp, &bytes, &key, &overlapped, INFINITE);
		if (NULL == overlapped) break;
            
        ((HNetIo*)overlapped)->Completion(bytes);
    }

    LOG_INFO_SYSTEM(L"end thread. id=\"%d\", name=\"%s\"", GetCurrentThreadId(), name);

    return 0;
}

unsigned WINAPI WorkerEx(void* arg)
{
    HANDLE           iocp = *(HANDLE*)arg;
    OVERLAPPED_ENTRY entries[32];
    ULONG            count;

    const wchar_t* name = L"network io ex";

    LOG_INFO_SYSTEM(L"begin thread. id=\"%d\", name=\"%s\"", GetCurrentThreadId(), name);

    while (true)
    {
        GetQueuedCompletionStatusEx(iocp, entries, _countof(entries), &count, INFINITE, TRUE);

        for (ULONG i = 0; i < count; i++)
        {
            if (NULL == entries[i].lpOverlapped)
                continue;

            ((HNetIo*)entries[i].lpOverlapped)->Completion(entries[i].dwNumberOfBytesTransferred);
        }
    }

    LOG_INFO_SYSTEM(L"end thread. id=\"%d\", name=\"%s\"", GetCurrentThreadId(), name);

    return 0;
}

unsigned WINAPI WorkerJob(void* arg)
{
    HANDLE         iocp       = *(HANDLE*)arg;
    DWORD          bytes      = 0;
    ULONG_PTR      key        = 0;
    OVERLAPPED*    overlapped = NULL;
    const wchar_t* name       = L"network io";

    LOG_INFO_SYSTEM(L"begin thread. id=\"%d\", name=\"%s\"", GetCurrentThreadId(), name);

    while (true)
    {
        GetQueuedCompletionStatus(iocp, &bytes, &key, &overlapped, INFINITE);
        if (NULL == overlapped) break;

        ((HNetIo*)overlapped)->CompletionJob(bytes);
    }

    LOG_INFO_SYSTEM(L"end thread. id=\"%d\", name=\"%s\"", GetCurrentThreadId(), name);

    return 0;
}
