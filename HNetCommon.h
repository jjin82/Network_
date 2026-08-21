#pragma once
#pragma warning(disable:4995)

#include <WinSock2.h>
#include <Mswsock.h>
#include <windows.h>
#include <new>
#include <process.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Mswsock.lib")

//■=============================================================================================■
//   API, LIB.
//■=============================================================================================■
#include "Common/Common.h"


//■=============================================================================================■
//   typedef.
//■=============================================================================================■
typedef __int32              NetId;


//■=============================================================================================■
//   define.
//■=============================================================================================■
#define HLIB                 HNET::LIB
#define HAPI                 HNET::API
#define HJOB                 HNET::JOB
#define HOPTION              HNET::OPTION
#define HSINGLETON           HNET::SINGLETON

#define HNET_DEFAULT_PORT    20000

#define HNET_HOST_LENTH      1024
#define HNET_NAME_LENTH      1024

#define T1(AAA)              template<typename AAA>
#define T2(AAA, BBB)         template<typename AAA, typename BBB>
#define TP(AAA)              template<typename AAA, typename p...>


//■=============================================================================================■
//   common class.
//■=============================================================================================■
class HNetHost
{
public:
    wchar_t _host[HNET_HOST_LENTH];
    USHORT  _port;
};

//■=============================================================================================■
//   성능 정보.
//■=============================================================================================■
class Performance
{
public:
    void RecvIo(int bytes)
    {
        if (++_recvIoCount > _recvIoCountMax)
        {
            _recvIoCountMax = _recvIoCount;
        }

        if ((_recvBytes += bytes) > _recvBytesMax)
        {
            _recvBytesMax = _recvBytes;
        }
    }

    void AddRecvCount(int addCount)
    {
        if ((_recvCount += addCount) > _recvCountMax)
        {
            _recvCountMax = _recvCount;
        }
    }

    void NextRecvInfo(OUT char*& IOCount, OUT char*& count, OUT char*& maxCount, OUT char*& bytes, OUT char*& maxBytes)
    {
        thread_local char t_ioCount[256]   = "";
        thread_local char t_count[256]     = "";
        thread_local char t_maxCount[256]  = "";
        thread_local char t_byte[256]      = "";
        thread_local char t_maxByte[256]   = "";

        HNET::API::StringPrintf(t_ioCount,  "%lld", _recvIoCount);
        HNET::API::StringPrintf(t_count,    "%lld", _recvCount);
        HNET::API::StringPrintf(t_maxCount, "%lld", _recvCountMax);
        HNET::API::StringPrintf(t_byte,     "%s",   ConvertByte(_recvBytes));
        HNET::API::StringPrintf(t_maxByte,  "%s",   ConvertByte(_recvBytesMax));

        IOCount         = t_ioCount;
        count           = t_count;
        maxCount        = t_maxCount;
        bytes           = t_byte;
        maxBytes        = t_maxByte;

        _recvIoCount    = 0;
        _recvCount      = 0;
        _recvBytes      = 0;
    }

    void SendIo(int bytes)
    {
        if (++_sendIoCount > _sendIoCountMax)
        {
            _sendIoCountMax = _sendIoCount;
        }

        if ((_sendBytes += bytes) > _sendBytesMax)
        {
            _sendBytesMax = _sendBytes;
        }
    }

    void AddSendCount(int addCount)
    {
        if ((_sendCount += addCount) > _sendCountMax)
        {
            _sendCountMax = _sendCount;
        }
    }

    void NextSendInfo(OUT char*& IOCount, OUT char*& count, OUT char*& maxCount, OUT char*& bytes, OUT char*& maxBytes)
    {
        thread_local char t_ioCount[256]   = "";
        thread_local char t_count[256]     = "";
        thread_local char t_maxCount[256]  = "";
        thread_local char t_byte[256]      = "";
        thread_local char t_maxByte[256]   = "";

        HNET::API::StringPrintf(t_ioCount,  "%lld", _sendIoCount);
        HNET::API::StringPrintf(t_count,    "%lld", _sendCount);
        HNET::API::StringPrintf(t_maxCount, "%lld", _sendCountMax);
        HNET::API::StringPrintf(t_byte,     "%s",   ConvertByte(_sendBytes));
        HNET::API::StringPrintf(t_maxByte,  "%s",   ConvertByte(_sendBytesMax));

        IOCount         = t_ioCount;
        count           = t_count;
        maxCount        = t_maxCount;
        bytes           = t_byte;
        maxBytes        = t_maxByte;

        _sendIoCount    = 0;
        _sendCount      = 0;
        _sendBytes      = 0;
    }

    char* ConvertByte(int64_t bytes)
    {
        thread_local char t_dummy[256] = "";

        if (bytes >= 1073741824)    { HNET::API::StringPrintf(t_dummy, "%lld GB", (bytes / 1073741824)); }
        else if (bytes >= 1048576)  { HNET::API::StringPrintf(t_dummy, "%lld MB", (bytes / 1048576)); }
        else if (bytes >= 1024)     { HNET::API::StringPrintf(t_dummy, "%lld KB", (bytes / 1024)); }
        else                        { HNET::API::StringPrintf(t_dummy, "%lld B", bytes); }

        return t_dummy;
    }

public:
    int64_t _recvIoCount;
    int64_t _recvIoCountMax;
    int64_t _recvCount;
    int64_t _recvCountMax;
    int64_t _recvBytes;
    int64_t _recvBytesMax;

    int64_t _sendIoCount;
    int64_t _sendIoCountMax;
    int64_t _sendCount;
    int64_t _sendCountMax;
    int64_t _sendBytes;
    int64_t _sendBytesMax;
};

extern Performance g_performance;

namespace HNET
{
    namespace PERFORMANCE
    {
        void NextRecvInfo(OUT char*& IOCount, OUT char*& count, OUT char*& maxCount, OUT char*& bytes, OUT char*& maxBytes);
        void NextSendInfo(OUT char*& IOCount, OUT char*& count, OUT char*& maxCount, OUT char*& bytes, OUT char*& maxBytes);
    }
}

