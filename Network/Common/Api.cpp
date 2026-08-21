#pragma warning(disable:4100 4091)

#include <WinSock2.h>
#include <atlsocket.h>

#include "Api.h"
#include <Psapi.h>
#include <dbghelp.h>
#include <vector>
#include <chrono>
#include <thread>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "dbghelp.lib")

#define HAPI_STR_ARRAY_COUNT 32   // 기본 문자열 버퍼 개수.
#define HAPI_STR_LENTH       8192 // 기본 문자열 버퍼 길이.

enum PRINT_COLOR
{
    DEFAULT = 0x0007,
    GREEN   = 0x0002,
    RED     = 0x0004,
    YELLOW  = 0x0006,
    BLUE    = 0x0003
};

SystemLogger* Log()
{
    static SystemLogger* log = HNET::SINGLETON::Singleton<SystemLogger>();
    return log;
}

JobHander* Job()
{
    static JobHander* job = HNET::SINGLETON::Singleton<JobHander>();
    return job;
}

LONG __stdcall ExceptionHandler(EXCEPTION_POINTERS* e);

// 시스템 로그 기록 여부.
bool recordSystemLog = true;

// 로그 파일 폴더.
wchar_t logFilePath[MAX_PATH] = L"Log";

namespace HNET
{
    namespace SINGLETON
    {
        void SystemLogReference()
        {
            Singleton<SystemLogger>()->Reference();
        }

        void SystemLogRelease()
        {
            Singleton<SystemLogger>()->Release();
        }
    }
}

namespace HNET
{
    namespace OPTION
    {
        void SetSystemLog(bool b)
        {
            recordSystemLog = b;
        }

        bool SetLogFilePath(const wchar_t* path)
        {
            return (0 == wcsncpy_s(logFilePath, _countof(logFilePath), path, _TRUNCATE));
        }
    }
}

namespace HNET
{
    namespace JOB
    {
        void Update()
        {
            Job()->Execute();
        }

        void Post(unsigned int id, std::function<void()> f, DWORD delay)
        {
            Job()->Post(id, f, delay);
        }

        void Post(std::function<void()> f, DWORD delay)
        {
            Job()->Post(f, delay);
        }
    }
}

namespace HNET
{
    namespace API
    {
        void Sleep(int milliseconds)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
        }

        void MiniDump()
        {
            // 미니덤프. (디버거가 걸려있지 않다면 예외 발생 시 MiniDump 호출해준다.)
            if (NULL == SetUnhandledExceptionFilter(ExceptionHandler))
                MSG_BOX_DETAIL(L"error");
        }

        void Print(PRINT_COLOR color, SHORT x, SHORT y, const wchar_t* format, va_list& Args)
        {
            static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

            COORD   Coor                   = { x, y };
            WORD    attr[HAPI_STR_LENTH]   = { 0, };
            
            wchar_t wString[HAPI_STR_LENTH] = L"";
            API::StringVPrintf(wString, format, Args);
                        
            const char* string = API::WCharToChar(wString);
            DWORD lenth = (DWORD)strnlen_s(string, HAPI_STR_LENTH);

            for (WORD count = 0; count < lenth; ++count)
                attr[count] = (WORD)color;

            char clean[1024];
            ZeroMemory(clean, sizeof(clean));

            DWORD dw;
            WriteConsoleOutputAttribute(hConsole, attr, lenth, Coor, &dw);
            WriteConsoleOutputCharacterA(hConsole, clean, sizeof(clean), Coor, &dw);
            WriteConsoleOutputCharacterA(hConsole, string, lenth, Coor, &dw);
        }
        #define PRINT(COLOR) va_list Args; va_start(Args, format); Print(COLOR, x, y, format, Args); va_end(Args);
        void Print(SHORT x, SHORT y, const wchar_t* format, ...)  { PRINT(DEFAULT); }
        void PrintR(SHORT x, SHORT y, const wchar_t* format, ...) { PRINT(RED);     }
        void PrintG(SHORT x, SHORT y, const wchar_t* format, ...) { PRINT(GREEN);   }
        void PrintB(SHORT x, SHORT y, const wchar_t* format, ...) { PRINT(BLUE);    }
        void PrintY(SHORT x, SHORT y, const wchar_t* format, ...) { PRINT(YELLOW);  }

	    bool LastError(const wchar_t* format, ...)
	    {
            DWORD errorCode  = GetLastError();

            static thread_local wchar_t error[1024] = L"";
		    DWORD lenth = FormatMessageW((FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS)
                                        ,NULL
                                        ,errorCode
                                        ,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
                                        ,error
                                        ,sizeof(error)
                                        ,NULL);
            if (0 == lenth) 
                return false;

		    // error 문자열에서 '\r' 부터 제거.
		    error[lenth - 2] = '\0';

            if (format)
            {
                wchar_t string[1024] = L"";

                va_list Args;
                va_start(Args, format);
                    StringVPrintf(string, format, Args);
                va_end(Args);

                Log()->Write(LOG_TYPE_LAST_ERROR, L"[%d]=\"%s\", %s", errorCode, error, string);
            }
            else
            {
		        Log()->Write(LOG_TYPE_LAST_ERROR, L"[%d]=\"%s\"", errorCode, error);
            }

            return false;
	    }

		bool CreateFolder(const wchar_t* path)
		{
			// 디렉토리 유무 확인.
			if (PathFileExistsW(path))
				return true;

			// Log 디렉토리가 없으면 생성.
			if (CreateDirectoryW(path, NULL))
				return true;

			return false;
		}

		void OutputDebugText(const wchar_t* string)
		{
		#ifdef _DEBUG
			wchar_t s[1024] = L"";
            StringPrintf(s, L"%s %s", L"▶", string);
			OutputDebugStringW(s);
		#endif
		}

        void MsgBox(const wchar_t* title, const wchar_t* format, ...)
        {
            if (NULL == title || NULL == format)
                return;

            wchar_t error[4096] = L"";
			
			va_list Args; 
			va_start(Args, format);
                StringVPrintf(error, format, Args);
			va_end(Args);
           
            MessageBoxW(NULL, error, title, 0);
        }
		
        const wchar_t* LocalIp()
        {
            WSAStart();

            char hostName[255];
            if (0 != gethostname(hostName, sizeof(hostName)))
            {
                WSAEnd();
                MSG_BOX_DETAIL(L"error");
                return L"";
            }

            WSAEnd();

            return HostToIp(CharToWChar(hostName));
        }

        const wchar_t* HostToIp(const wchar_t* host)
        {
            WSAStart();

            static thread_local wchar_t t_ip[1024];
            ZeroMemory(t_ip, sizeof(t_ip));

            ADDRINFOW hint;
            ZeroMemory(&hint, sizeof(hint));
            hint.ai_family   = AF_UNSPEC;
            hint.ai_socktype = SOCK_STREAM;
            hint.ai_protocol = IPPROTO_TCP;

            ADDRINFOW* result = NULL;
            GetAddrInfoW(host, NULL, &hint, &result);

            DWORD ipLenth;
            for (ADDRINFOW* p = result; p != NULL; p = p->ai_next)
            {
                switch (p->ai_family)
                {
                case AF_UNSPEC:
                    LOG_INFO(L"Unspecified( %s )", host);
                    break;
                case AF_INET:
                    LOG_INFO(L"AF_INET (IPv4): %s", host);
                    WSAAddressToStringW(p->ai_addr, (DWORD)p->ai_addrlen, NULL, t_ip, &ipLenth);
                    break;
                case AF_INET6:
                    LOG_INFO(L"AF_INET6 (IPv6): %s", host);
                    WSAAddressToStringW(p->ai_addr, (DWORD)p->ai_addrlen, NULL, t_ip, &ipLenth);
                    break;
                }
            }
            
            WSAEnd();

            return t_ip;
        }

	    void BreakMemoryLeak(int number)
        {
            _CrtSetBreakAlloc(number);
        }

        const wchar_t* Directory()
        {
            static wchar_t path[MAX_PATH] = L"\0";
            GetCurrentDirectoryW(MAX_PATH, path);

            return path;
        }

        wchar_t DiskDrive()
        {
            return *Directory();
        }

        const wchar_t* LogFilePath()
        {
            return logFilePath;
        }

        const wchar_t* ExeFileName()
        {
            static wchar_t* fileName           = NULL;
            static wchar_t  fullFileName[1024] = L"";

            if (NULL == fileName)
            {
                if (0 == GetModuleFileNameW(NULL, fullFileName, _countof(fullFileName)))
                    return NULL;
                
                for (int i = 0; '\0' != fullFileName[i]; ++i)
                {
                    if (fullFileName[i] != L'\\' && fullFileName[i] != L'/')
                        continue;

                    fileName = &fullFileName[i + 1];
                }
            }

            return fileName;
        }

        SIZE_T MemoryUsage()
        {
            PROCESS_MEMORY_COUNTERS	meminfo;
            GetProcessMemoryInfo(GetCurrentProcess(), &meminfo, sizeof(PROCESS_MEMORY_COUNTERS));

            if (meminfo.PagefileUsage > MemoryUsageMax())
				MemoryUsageMax() = meminfo.PagefileUsage;

            return meminfo.PagefileUsage;
        }

        SIZE_T& MemoryUsageMax()
        { 
			static SIZE_T s_maxMemory = 0;
			return s_maxMemory;
        } 

	    int HddFreeSpace(wchar_t drive)
	    {
            wchar_t disk[4] = L"x:\\";
            disk[0]         = drive;

		    ULARGE_INTEGER avail, total, free;
		    if (FALSE == GetDiskFreeSpaceExW(disk, &avail, &total, &free))
            {
                LOG_LAST_ERROR_DETAIL_SYSTEM();
                return 0;
            }

		    // MByte 단위.
		    //int totalMegaByte = (int)(total.QuadPart >> 20);
		    int freeMegaByte  = (int)(free.QuadPart  >> 20);

		    return freeMegaByte;
	    }

	    DWORD ProcessorCount()
	    {
		    SYSTEM_INFO sysInfo;
		    GetSystemInfo(&sysInfo);

		    return sysInfo.dwNumberOfProcessors;
	    }

        const wchar_t* WindowCaption()
        {
            static wchar_t s_title[1024] = L"";
            
            if ('\0' == s_title[0])
            {
                if (0 == GetWindowTextW(ProcessIdToHwnd(), s_title, _countof(s_title)))
                {
                    LOG_LAST_ERROR_DETAIL_SYSTEM();
                    return NULL;
                }
            }

            return s_title;
        }

        HWND WindowNameToHwnd(const wchar_t* windowName)
        {
            return FindWindowW(NULL, windowName);
        }

        HWND ProcessIdToHwnd(ULONG processId)
        {   
            HWND hwnd = FindWindowW(NULL, NULL); // 최상위 윈도우 핸들.
            while (hwnd)
            {
                // 최상위 핸들 체크.
                if (NULL == GetParent(hwnd))
                {
                    // 핸들의 프로세스 아이디 획득 후 비교.
                    DWORD pId;
                    GetWindowThreadProcessId(hwnd, &pId);
                    if (processId == pId)
                        break;
                }
                
                // 다음 윈도우 핸들.
                hwnd = GetWindow(hwnd, GW_HWNDNEXT);
            }
            
            return hwnd;
        }

        time_t ConvertToTime(const SYSTEMTIME& time)
        {
	        tm t;
	        ZeroMemory(&t, sizeof(t));

	        t.tm_year = time.wYear  - 1900;
	        t.tm_mon  = time.wMonth - 1;
	        t.tm_mday = time.wDay;
	        t.tm_hour = time.wHour;
	        t.tm_min  = time.wMinute;
	        t.tm_sec  = time.wSecond;

	        return mktime(&t);
        }

        FILETIME ConvertToFileTime(const SYSTEMTIME& time)
        {
	        FILETIME fileTime = { 0, 0 };
            SystemTimeToFileTime(&time, &fileTime);
	        
            return fileTime;
        }

        SYSTEMTIME ConvertToSystemTime(FILETIME time)
        {
            SYSTEMTIME systemTime;
            FileTimeToSystemTime(&time, &systemTime);

            return systemTime;
        }

        SYSTEMTIME ConvertToSystemTime(const time_t time)
        {
            SYSTEMTIME systemTime;
            ZeroMemory(&systemTime, sizeof(systemTime));

            struct tm t;
            if (0 != localtime_s(&t, &time))
                return systemTime;

            systemTime.wYear      = (WORD)t.tm_year + 1900;
            systemTime.wMonth     = (WORD)t.tm_mon + 1;
            systemTime.wDayOfWeek = (WORD)t.tm_wday;
            systemTime.wDay       = (WORD)t.tm_mday;
            systemTime.wHour      = (WORD)t.tm_hour;
            systemTime.wMinute    = (WORD)t.tm_min;
            systemTime.wSecond    = (WORD)t.tm_sec;

            return systemTime;
        }

        bool BetweenTime(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime)
        {
	        SYSTEMTIME curTime;
	        GetLocalTime(&curTime);

	        UINT64 cur, start, end;
            if (FALSE == SystemTimeToFileTime(&curTime,   (FILETIME*)&cur))   return false;
            if (FALSE == SystemTimeToFileTime(&startTime, (FILETIME*)&start)) return false;
            if (FALSE == SystemTimeToFileTime(&endTime,   (FILETIME*)&end))   return false;

	        if (start <= cur && end >= cur)
		        return true;
	        else
		        return false;
        }

        bool TimeOver(const SYSTEMTIME& time)
        {
            SYSTEMTIME curTime;
            GetLocalTime(&curTime);

            UINT64 cur, target;
            if (FALSE == SystemTimeToFileTime(&curTime, (FILETIME*)&cur))    return false;
            if (FALSE == SystemTimeToFileTime(&time,    (FILETIME*)&target)) return false;
	
	        return (cur < target);
        }

        WORD DaysPassed(const SYSTEMTIME& time)
        {
	        const static time_t day = 60 * 60 * 24; // 하루.

	        return (WORD)((::time(NULL) - ConvertToTime(time)) / day);
        }

        WORD HoursPassed(const SYSTEMTIME& time)
        {
	        const static time_t hour = 60 * 60; // 1 시간.

	        return (WORD)((::time(NULL) - ConvertToTime(time)) / hour);
        }

        WORD MinutesPassed(const SYSTEMTIME& time)
        {
	        const static time_t minute = 60; // 1분.

	        return (WORD)((::time(NULL) - ConvertToTime(time)) / minute);
        }

        const wchar_t* CharToWChar(const char* string)
        {
            static thread_local int     t_index;
            static thread_local wchar_t t_string[HAPI_STR_ARRAY_COUNT][HAPI_STR_LENTH];

            int index = ++t_index % HAPI_STR_ARRAY_COUNT;
            if (0 == MultiByteToWideChar(CP_ACP, 0, string, -1, t_string[index], _countof(t_string[index])))
                return L"";

            return t_string[index];
        }

        const char* WCharToChar(const wchar_t* string)
        {
            static thread_local int  t_index;
            static thread_local char t_string[HAPI_STR_ARRAY_COUNT][HAPI_STR_LENTH];

            int index = ++t_index % HAPI_STR_ARRAY_COUNT;
            if (0 == WideCharToMultiByte(CP_ACP, 0, string, -1, t_string[index], _countof(t_string[index]), NULL, NULL))
                return "";

            return t_string[index];
        }

        const char* CharToUtf8(const char* string)
        {
            static thread_local char t_string[HAPI_STR_LENTH];

            return t_string;
        }

        const char* WCharToUtf8(const wchar_t* string)
        {
            static thread_local char t_string[HAPI_STR_LENTH];
            
            return t_string;
        }

        size_t StringLenth(const char* string)
        {
            if (NULL == string)
                return false;

            return strnlen_s(string, HAPI_STR_LENTH);
        }

        size_t StringLenth(const wchar_t* string)
        {
            if (NULL == string)
                return false;

            return wcsnlen_s(string, HAPI_STR_LENTH);
        }

        bool StringCompare(const char* string1, const char* string2)
        {
            if (NULL == string1 || NULL == string2)
                return false;

            if (string1 == string2)
                return true;

            return (0 == strncmp(string1, string2, HAPI_STR_LENTH));
        }

        bool StringCompare(const wchar_t* string1, const wchar_t* string2)
        {
            if (NULL == string1 || NULL == string2)
                return false;

            if (string1 == string2)
                return true;

            return (0 == wcsncmp(string1, string2, HAPI_STR_LENTH));
        }

        const char* StringFormat(const char* format, ...)
        {
            static thread_local char t_string[4096];
            t_string[0] = '\0';

            if (NULL == format)
                return "";

            va_list Args;
            va_start(Args, format);
                _vsnprintf_s(t_string, _countof(t_string), _TRUNCATE, format, Args);
            va_end(Args);

            return t_string;
        }

        const wchar_t* StringFormat(const wchar_t* format, ...)
        {
            static thread_local wchar_t t_string[4096];
            t_string[0] = '\0';

            if (NULL == format)
                return L"";

            va_list Args;
            va_start(Args, format);
                _vsnwprintf_s(t_string, _countof(t_string), _TRUNCATE, format, Args);
            va_end(Args);

            return t_string;
        }
    }
}

ULONG g_wsaRefCount = 0;
void WSAStart()
{
    if (InterlockedExchangeAdd(&g_wsaRefCount, 1))
        return;

    WSADATA	wsaData;
    if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        WSAEnd();
        MSG_BOX(L"error", L"WSAStartup.");
        return;
    }

    if (2 != LOBYTE(wsaData.wVersion) || 2 != HIBYTE(wsaData.wVersion))
    {
        WSAEnd();
        MSG_BOX(L"error", L"different versions");
        return;
    }

    // setlocale을 지정하지 않으면 프로젝트 속성 항목의 [문자집합:설정 안함]에서는 정상적으로 
    // 한글이 나오지만 [문자집합:유니코드 사용]에서는 한글을 볼 수 없게 된다.
    _wsetlocale(LC_ALL, L"");

    // 매번 다른 시드를 갖도록 세팅.
    srand((unsigned int)time(NULL));

    // 메모리 누수 체크.(debug version only)
    _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
}

void WSAEnd()
{
    if (InterlockedDecrement(&g_wsaRefCount))
        return;

    WSACleanup();
}

LONG __stdcall ExceptionHandler(EXCEPTION_POINTERS* e)
{
    const wchar_t* folder = L"Dump";

    // 디렉토리 유무 확인.
    if (FALSE == PathFileExistsW(folder))
    {
        // dump 디렉토리가 없으면 생성.
        if (FALSE == CreateDirectoryW(folder, NULL))
        {
            MSG_BOX_DETAIL(L"error");
            return false;
        }
    }

    LONG retval = EXCEPTION_CONTINUE_SEARCH;

    // 배열 인덱스 초과는 정상처리로 돌린다.
    if (EXCEPTION_ARRAY_BOUNDS_EXCEEDED == e->ExceptionRecord->ExceptionCode)
        retval = EXCEPTION_CONTINUE_EXECUTION;

    CAtlString fileName;
    fileName  = folder;
    fileName += L"/";
    fileName += HNET::API::ExeFileName();

    SYSTEMTIME SystemTime;
    GetLocalTime(&SystemTime);

    wchar_t dumpFileName[MAX_PATH] = L"";
    HNET::API::StringPrintf(dumpFileName, L"%s [%04d-%02d-%02d, %02d'%02d'%02d](%p).dmp", 
        fileName.GetString(),
        SystemTime.wYear,
        SystemTime.wMonth,
        SystemTime.wDay,
        SystemTime.wHour,
        SystemTime.wMinute,
        SystemTime.wSecond,
        e->ExceptionRecord->ExceptionAddress);

    HANDLE file = CreateFileW(dumpFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE != file)
    {
        MINIDUMP_EXCEPTION_INFORMATION eInfo;
        eInfo.ThreadId = GetCurrentThreadId();
        eInfo.ExceptionPointers = e;
        eInfo.ClientPointers = FALSE;

        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, MiniDumpWithFullMemory, &eInfo, NULL, NULL);

        CloseHandle(file);
    }

    return retval;
}

HNET::LIB::Logger* SystemLog()
{
    return recordSystemLog ? Log() : NULL; // 로그 기록 설정값 여부에 따라 처리.
}