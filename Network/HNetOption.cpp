#include "HNetOption.h"
#include "HNetPacket.h"

HNetOption::HNetOption()
{
    // ini file.
    HAPI::StringCopy(_fileName, L"");
    HAPI::StringCopy(_path, L"");
    
    // identity.
    _id = 1;
    HAPI::StringCopy(_name, L"HNet");
    
    // info.
    _port = HNET_DEFAULT_PORT;
}

bool HNetOption::OpenIniFile(const wchar_t* iniFileName)
{
	if (FALSE == PathFileExistsW(iniFileName))
	{
        MSG_BOX(L"error", L"\"%s\" ini file does not exist", iniFileName); 
		return false;
	}

	// file name.
	HAPI::StringCopy(_fileName, iniFileName);

	// file full name.
	HAPI::StringCopy(_path, HAPI::Directory());
    HAPI::StringCat(_path,  L"\\");
    HAPI::StringCat(_path,  _fileName);

	// check.
    wchar_t sections[4096];
	if (0 == GetPrivateProfileSectionNamesW(sections, _countof(sections), _path))
	{
		MSG_BOX(L"error", L"file=\"%s\", not ANSI or UNICODE encoding file", _fileName);
		return false;
	}

	LOG_INFO_SYSTEM(L"◆ open \"%s\" option file", _fileName);

	return true;
}

bool HNetOption::LoadOption(HNET_OPTION type, const wchar_t* section, const wchar_t* key)
{ 
    wchar_t temp[1024];
	if (0 == GetPrivateProfileStringW(section, key, L"", temp, _countof(temp), _path))
	{
        MSG_BOX(L"error", L"file=\"%s\", section=\"%s\", key=\"%s\"", _fileName, section, key);
		return false;
	}

	switch(type)
	{
    case HNET_OPTION::HOST:      HAPI::StringCopy(_host, temp);   break;
	case HNET_OPTION::ID:        _id       = _wtoi(temp);         break;
	case HNET_OPTION::PORT:      _port     = (USHORT)_wtoi(temp); break;
	case HNET_OPTION::NAME:      HAPI::StringCopy(_name, temp);   break;
	}

	LOG_INFO_SYSTEM(L"◆ option file=\"%s\", section=\"%s\", key=\"%s\", value=\"%s\"", _fileName, section, key, temp);
        
    return true; 
}

void HNetOption::Host(const wchar_t* host)
{ 
    if (NULL == host)
        return;

    if ('\0' == host[0])
        return;

    HAPI::StringCopy(_host, host);
}

void HNetOption::Port(USHORT port)
{
    _port = port;
}
        
void HNetOption::Id(int id)
{ 
    _id = id; 
}

void HNetOption::Name(const wchar_t* name)
{ 
    HAPI::StringCopy(_name, name); 
}


//■=============================================================================================■
//   HNetConnectOption.
//■=============================================================================================■
HNetConnectOption::HNetConnectOption()
{
    HAPI::StringCopy(_name, L"HNetConnector");
    HAPI::StringCopy(_host, L"127.0.0.1");
}


//■=============================================================================================■
//   HNetAcceptOption.
//■=============================================================================================■
HNetAcceptOption::HNetAcceptOption()
{
    HAPI::StringCopy(_name, L"HNetAcceptor");
    _sessionCount = 5000;
    _threadCount  = 1;

    SOCKADDR_IN6 AddrIn;
    AddrIn.sin6_addr = in6addr_any;
    InetNtopW(AF_INET6, &AddrIn.sin6_addr, _host, _countof(_host));
}

bool HNetAcceptOption::LoadOption(HNET_OPTION type, const wchar_t* section, const wchar_t* key)
{
    wchar_t temp[1024];
    if (0 == GetPrivateProfileStringW(section, key, L"", temp, _countof(temp), _path))
    {
        MSG_BOX(L"error", L"file=\"%s\", section=\"%s\", key=\"%s\"", _fileName, section, key);
        return false;
    }

    switch (type)
    {
    case HNET_OPTION::HOST:          HAPI::StringCopy(_host, temp);       break;
    case HNET_OPTION::ID:            _id           = _wtoi(temp);         break;
    case HNET_OPTION::PORT:          _port         = (USHORT)_wtoi(temp); break;
    case HNET_OPTION::NAME:          HAPI::StringCopy(_name, temp);       break;
    case HNET_OPTION::SESSION_COUNT: _sessionCount = (USHORT)_wtoi(temp); break;
    case HNET_OPTION::THREAD_COUNT:  _threadCount  = (USHORT)_wtoi(temp); break;
    }

    LOG_INFO_SYSTEM(L"◆ option file=\"%s\", section=\"%s\", key=\"%s\", value=\"%s\"", _fileName, section, key, temp);

    return true;
}
        
void HNetAcceptOption::SessionCount(USHORT count)
{ 
    _sessionCount = count;
}
        
void HNetAcceptOption::ThreadCount(USHORT count)
{ 
    // 최대 쓰레드 개수.
    static USHORT maxThreadCount = (USHORT)(HAPI::ProcessorCount() * 3);
    
    (count > maxThreadCount) ? count = maxThreadCount : _threadCount = count;
}

