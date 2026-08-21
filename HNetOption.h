#pragma once
#pragma warning(disable:4100)

#include "HNetCommon.h"

enum class HNET_OPTION
{
	ID,
	NAME,
    HOST,
    PORT,
    SESSION_COUNT,
	THREAD_COUNT
};

//■=============================================================================================■
//   option.
//■=============================================================================================■
class HNetOption
{
public:
    HNetOption();

    bool OpenIniFile(const wchar_t* iniFileName);
    virtual bool LoadOption(HNET_OPTION type, const wchar_t* section, const wchar_t* key);

    void Id(int id);
    void Name(const wchar_t* name);
    void Host(const wchar_t* host);
    void Port(USHORT port);

protected:
    wchar_t _fileName[1024];           // ini 파일 이름.
    wchar_t _path[1024];               // 전체 경로.

    int     _id;                       // Id.
    wchar_t _name[HNET_NAME_LENTH];    // name.
    wchar_t _host[HNET_HOST_LENTH];    // host.
    USHORT  _port;                     // port.
};


//■=============================================================================================■
//   HNetConnectOption.
//■=============================================================================================■
class HNetConnectOption : public HNetOption
{
    friend class HNetConnector;

public:
    HNetConnectOption();
};


//■=============================================================================================■
//   HNetAcceptOption.
//■=============================================================================================■
class HNetAcceptOption : public HNetOption
{
    friend class HNetAcceptor;
#ifdef HNETWORK_FULL_SOURCE
    friend class HNetWsAcceptor;
#endif

public:
    HNetAcceptOption();

    bool LoadOption(HNET_OPTION type, const wchar_t* section, const wchar_t* key) override;

    void SessionCount(USHORT count);
    void ThreadCount(USHORT count);

private:
    USHORT _sessionCount;   // 접속 개수
    USHORT _threadCount;    // 쓰레드 개수
};

namespace HNET
{
    typedef HNetConnectOption       ConnectOption;
    typedef HNetAcceptOption        AcceptOption;
}