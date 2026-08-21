#pragma once

#include "Common.h"

#define _W(X) L##X
#define __W(X) _W(X)

//■=============================================================================================■
//   LOG.
//■=============================================================================================■
class SystemLogger : public HNET::LIB::Logger {};
SystemLogger* Log();

#define LOG_INFO(FORMAT, ...)        Log()->Write(LOG_TYPE_INFO,       FORMAT, __VA_ARGS__)
#define LOG_ERROR(FORMAT, ...)       Log()->Write(LOG_TYPE_ERROR,      FORMAT, __VA_ARGS__)
#define LOG_CRITICAL(FORMAT, ...)    Log()->Write(LOG_TYPE_CRITICAL,   FORMAT, __VA_ARGS__)
#define LOG_WARNING(FORMAT, ...)     Log()->Write(LOG_TYPE_WARNING,    FORMAT, __VA_ARGS__)
#define LOG_FAIL(FORMAT, ...)        Log()->Write(LOG_TYPE_FAIL,       FORMAT, __VA_ARGS__)
#define LOG_LAST_ERROR(FORMAT, ...)  Log()->Write(LOG_TYPE_LAST_ERROR, FORMAT, __VA_ARGS__)

#ifdef _DEBUG
    #define LOG_INFO_DEBUG(FORMAT, ...)       LOG_INFO(FORMAT,       __VA_ARGS__)
    #define LOG_ERROR_DEBUG(FORMAT, ...)      LOG_ERROR(FORMAT,      __VA_ARGS__)
    #define LOG_CRITICAL_DEBUG(FORMAT, ...)   LOG_CRITICAL(FORMAT,   __VA_ARGS__)
    #define LOG_WARNING_DEBUG(FORMAT, ...)    LOG_WARNING(FORMAT,    __VA_ARGS__)
    #define LOG_FAIL_DEBUG(FORMAT, ...)       LOG_FAIL(FORMAT,       __VA_ARGS__)
    #define LOG_LAST_ERROR_DEBUG(FORMAT, ...) LOG_LAST_ERROR(FORMAT, __VA_ARGS__)
#else
    #define LOG_INFO_DEBUG(FORMAT, ...)
    #define LOG_ERROR_DEBUG(FORMAT, ...)
    #define LOG_CRITICAL_DEBUG(FORMAT, ...)
    #define LOG_WARNING_DEBUG(FORMAT, ...)
    #define LOG_FAIL_DEBUG(FORMAT, ...)
    #define LOG_LAST_ERROR_DEBUG(FORMAT, ...)
#endif

namespace HNET
{
    namespace SINGLETON
    {
        void SystemLogReference();
        void SystemLogRelease();
    }
}

namespace HNET
{
    namespace OPTION
    {
        //■=============================================================================================■
        //   HNetwork 시스템의 로그를 기록 안함.
        //■=============================================================================================■
        void SetSystemLog(bool b);

        
        //■=============================================================================================■
        //   로그 파일 폴더 설정.
        //■=============================================================================================■
        bool SetLogFilePath(const wchar_t* path);
    }
}

class JobHander : public Job {};
JobHander* Job();

namespace HNET
{
    namespace JOB
    {
        void Update();

        void Post(unsigned int id, std::function<void()> f /* = [](){ } */, DWORD delay = 0);
        void Post(std::function<void()> f /* = [](){ } */, DWORD delay = 0);
    }
}

namespace HNET
{
    namespace API
    {
        void Sleep(int milliseconds);
        
        //■=============================================================================================■
        //   미니 덤프.
        //■=============================================================================================■
        void MiniDump();


        //■=============================================================================================■
        //   콘솔 화면에 문자열 출력.
        //■=============================================================================================■
        void Print(SHORT x, SHORT y, const wchar_t* format, ...);
        void PrintR(SHORT x, SHORT y, const wchar_t* format, ...);
        void PrintG(SHORT x, SHORT y, const wchar_t* format, ...);
        void PrintB(SHORT x, SHORT y, const wchar_t* format, ...);
        void PrintY(SHORT x, SHORT y, const wchar_t* format, ...);


        //■=============================================================================================■
        //   마지막 에러를 문자열로 반환.(로그도 기록)
        //■=============================================================================================■
	    bool LastError(const wchar_t* format = NULL, ...);

        #define LAST_ERROR_DETAIL() HNET::API::LastError(L"function=\"%s\", line=\"%d\"", __W(__FUNCTION__), __LINE__)
        
		
		//■=============================================================================================■
		//   폴더 생성. (생성하거나 이미 존재 한다면 true 리턴.)
		//■=============================================================================================■
		bool CreateFolder(const wchar_t* path);
        

		//■=============================================================================================■
		//   디버그 모드에서 문자열 노출. ("▶" 구분 문자가 추가됨)
		//■=============================================================================================■
		void OutputDebugText(const wchar_t* string);


        //■=============================================================================================■
        //   메시지 박스.
        //■=============================================================================================■
        void MsgBox(const wchar_t* title, const wchar_t* format, ...);

        #define MSG_BOX(TITLE, FORMAT, ...) HNET::API::MsgBox(TITLE, FORMAT, __VA_ARGS__)
        #define MSG_BOX_DETAIL(TITLE)       HNET::API::MsgBox(TITLE, L"filename=\"%s\", line=\"%d\", function=\"%s\"", __W(__FILE__), __LINE__, __W(__FUNCTION__))


        //■=============================================================================================■
        //   호스트 아이피.
        //■=============================================================================================■
        const wchar_t* HostToIp(const wchar_t* host);

        
        //■=============================================================================================■
        //   로컬 아이피.
        //■=============================================================================================■
        const wchar_t* LocalIp();


	    //■=============================================================================================■
        //   메모리릭 브레이크 포인트 설정.
        //■=============================================================================================■
        void BreakMemoryLeak(int number);


        //■=============================================================================================■
        //   현재 디렉토리.
        //■=============================================================================================■
        const wchar_t* Directory();

        
        //■=============================================================================================■
        //   현재 드라이브.
        //■=============================================================================================■
        wchar_t DiskDrive();

        
        //■=============================================================================================■
        //   로그 파일 경로.
        //■=============================================================================================■
        const wchar_t* LogFilePath();
        

        //■=============================================================================================■
        //   실행 파일 이름.
        //■=============================================================================================■
        const wchar_t* ExeFileName();


        //■=============================================================================================■
        //   메모리 사용량.
        //■=============================================================================================■
        SIZE_T MemoryUsage();      // 현재 사용량.
        SIZE_T& MemoryUsageMax();  // 최대 사용량.


	    //■=============================================================================================■
        //   하드의 빈 공간.
        //■=============================================================================================■
	    int HddFreeSpace(wchar_t drive = DiskDrive());


	    //■=============================================================================================■
        //   프로세서 개수.
        //■=============================================================================================■
	    DWORD ProcessorCount();

        
        //■=============================================================================================■
        //   윈도우 캡션.
        //■=============================================================================================■
        const wchar_t* WindowCaption();


        //■=============================================================================================■
        //   윈도우 캡션으로 HWND 얻기.
        //■=============================================================================================■
        HWND WindowNameToHwnd(const wchar_t* windowName = WindowCaption());

        
        //■=============================================================================================■
        //   프로세스 아이디로 HWND 얻기.
        //■=============================================================================================■
        HWND ProcessIdToHwnd(ULONG processId = GetCurrentProcessId());

        
        //■=============================================================================================■
        //   시간.
        //■=============================================================================================■
        time_t     ConvertToTime(const SYSTEMTIME& time);                                    // SYSTEMTIME -> time_t.
        FILETIME   ConvertToFileTime(const SYSTEMTIME& time);                                // SYSTEMTIME -> FILETIME.
        SYSTEMTIME ConvertToSystemTime(FILETIME time);                                       // FILETIME   -> SYSTEMTIME.
        SYSTEMTIME ConvertToSystemTime(const time_t time);                                   // time_t     -> SYSTEMTIME.
        bool       BetweenTime(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime);      // '현재시간'이 '시작시간'과 '끝시간' 안에 있는지 확인.  
        bool       TimeOver(const SYSTEMTIME& time);                                         // 시간이 지났는지.
        WORD       DaysPassed(const SYSTEMTIME& time);                                       // 몇일이 지났는지.
        WORD       HoursPassed(const SYSTEMTIME& time);                                      // 몇시간이 지났는지.
        WORD       MinutesPassed(const SYSTEMTIME& time);                                    // 몇분이 지났는지.


        //■=============================================================================================■
        //   문자열 변환.
        //■=============================================================================================■
        const wchar_t* CharToWChar(const char* string);
        const char*    WCharToChar(const wchar_t* string);
        const char*    CharToUtf8(const char* string);
        const char*    WCharToUtf8(const wchar_t* string);

        //■=============================================================================================■
        //   문자열 처리.
        //■=============================================================================================■
		size_t StringLenth(const char* string);
		size_t StringLenth(const wchar_t* string);
        bool StringCompare(const char* string1, const char* string2);
        bool StringCompare(const wchar_t* string1, const wchar_t* string2);
        const char* StringFormat(const char* format, ...);
        const wchar_t* StringFormat(const wchar_t* format, ...);
        template<int N> bool StringCopy(OUT char (&dest)[N], const char* src);
        template<int N> bool StringCopy(OUT wchar_t (&dest)[N], const wchar_t* src);
        template<int N> bool StringCat(OUT char (&dest)[N], const char* src);
        template<int N> bool StringCat(OUT wchar_t (&dest)[N], const wchar_t* src);
        template<int N> bool StringPrintf(OUT char (&dest)[N], const char* format, ...);
        template<int N> bool StringPrintf(OUT wchar_t (&dest)[N], const wchar_t* format, ...);
        template<int N> bool StringVPrintf(OUT char (&dest)[N], const char* format, va_list& Args);
        template<int N> bool StringVPrintf(OUT wchar_t (&dest)[N], const wchar_t* format, va_list& Args);
    }
}

void WSAStart();
void WSAEnd();

HNET::LIB::Logger* SystemLog();
#define LOG_INFO_SYSTEM(FORMAT, ...)        SystemLog() ? SystemLog()->Write(LOG_TYPE_INFO,       FORMAT, __VA_ARGS__) : 0
#define LOG_ERROR_SYSTEM(FORMAT, ...)       SystemLog() ? SystemLog()->Write(LOG_TYPE_ERROR,      FORMAT, __VA_ARGS__) : 0
#define LOG_CRITICAL_SYSTEM(FORMAT, ...)    SystemLog() ? SystemLog()->Write(LOG_TYPE_CRITICAL,   FORMAT, __VA_ARGS__) : 0
#define LOG_WARNING_SYSTEM(FORMAT, ...)     SystemLog() ? SystemLog()->Write(LOG_TYPE_WARNING,    FORMAT, __VA_ARGS__) : 0
#define LOG_FAIL_SYSTEM(FORMAT, ...)        SystemLog() ? SystemLog()->Write(LOG_TYPE_FAIL,       FORMAT, __VA_ARGS__) : 0
#define LOG_LAST_ERROR_SYSTEM(FORMAT, ...)  SystemLog() ? SystemLog()->Write(LOG_TYPE_LAST_ERROR, FORMAT, __VA_ARGS__) : 0
#define LOG_LAST_ERROR_DETAIL_SYSTEM()      SystemLog() ? HNET::API::LastError(L"function( \"%s\" ), line( \"%d\" )", __W(__FUNCTION__), __LINE__) : 0

template<int N>
bool HNET::API::StringCopy(OUT char (&dest)[N], const char* src)
{
    dest[0] = '\0';

    if (dest == src || NULL == src)
        return false;

    return (0 == strncpy_s(dest, N, src, _TRUNCATE));
}

template<int N> 
bool HNET::API::StringCopy(OUT wchar_t (&dest)[N], const wchar_t* src)
{
    dest[0] = '\0';

    if (dest == src || NULL == src)
        return false;

    return (0 == wcsncpy_s(dest, N, src, _TRUNCATE));
}

template<int N>
bool HNET::API::StringCat(OUT char (&dest)[N], const char* src)
{
    dest[N - 1] = '\0';

    if (dest == src || NULL == src)
        return false;

    return (0 == strncat_s(dest, N, src, _TRUNCATE));
}

template<int N>
bool HNET::API::StringCat(OUT wchar_t (&dest)[N], const wchar_t* src)
{
    dest[N - 1] = '\0';

    if (dest == src || NULL == src)
        return false;

    return (0 == wcsncat_s(dest, N, src, _TRUNCATE));
}

template<int N>
bool HNET::API::StringPrintf(OUT char (&dest)[N], const char* format, ...)
{
    dest[0] = '\0';

    if (dest == format || NULL == format)
        return false;

    va_list Args;
    va_start(Args, format);
        bool result = (0 < _vsnprintf_s(dest, N, _TRUNCATE, format, Args));
    va_end(Args);

    return result;
}

template<int N>
bool HNET::API::StringPrintf(OUT wchar_t (&dest)[N], const wchar_t* format, ...)
{
    dest[0] = '\0';

    if (dest == format || NULL == format)
        return false;

    va_list Args;
    va_start(Args, format);
       bool result = (0 < _vsnwprintf_s(dest, N, _TRUNCATE, format, Args));
    va_end(Args);

    return result;
}

template<int N>
bool HNET::API::StringVPrintf(OUT char (&dest)[N], const char* format, va_list& Args)
{
    dest[0] = '\0';

    if (dest == format || NULL == format)
        return false;

    return (0 < _vsnprintf_s(dest, N, _TRUNCATE, format, Args));
}

template<int N>
bool HNET::API::StringVPrintf(OUT wchar_t (&dest)[N], const wchar_t* format, va_list& Args)
{
    dest[0] = '\0';

    if (dest == format || NULL == format)
        return false;

    return (0 < _vsnwprintf_s(dest, N, _TRUNCATE, format, Args));
}