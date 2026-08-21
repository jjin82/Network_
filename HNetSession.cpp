#include "HNet.h"
#include "HNetSession.h"

HNetSession::HNetSession(HNet* hnet, USHORT index)
    : RefCount(0)
    , HNetIoListen(this)
    , HNetIoConnect(this)
    , HNetIoReconnect(this)
    , HNetIoAccept(this)
    , HNetIoRecv(this)
    , HNetIoSend(this)
    , HNetIoDisconnect(this)
    , HNetIoWsAccept(this)
    , HNetIoWsRecv(this)
    , _create(false)
    , _socket(INVALID_SOCKET)
    , _hnet(hnet)
    , _netId(index)
{
    ZeroMemory(&_address, sizeof(_address));
    NewNetId();
}

HNetSession::~HNetSession()
{
	Close();
}

bool HNetSession::Create()
{
    if (true == _create)
        return true;

    if (false == HNetIoRecv::Create())
        return false;

    if (false == HNetIoWsRecv::Create())
        return false;

    _create = true;

    return true;
}

bool HNetSession::Initialize()
{
    if (false == HNetIoRecv::Initialize())
        return false;

    if (false == HNetIoSend::Initialize())
        return false;

    if (false == HNetIoDisconnect::Initialize())
        return false;

    return true;
}

bool HNetSession::InitializeWs()
{
    if (false == HNetIoWsRecv::Initialize())
        return false;

    if (false == HNetIoSend::Initialize())
        return false;

    if (false == HNetIoDisconnect::Initialize())
        return false;

    return true;
}

bool HNetSession::Open()
{
    if (0 == _address.ss_family)
        return false;

    return Open(_address.ss_family);
}

bool HNetSession::Open(ADDRESS_FAMILY af)
{
    if (INVALID_SOCKET != _socket)
        closesocket(_socket);

    _socket = WSASocketW(af, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (INVALID_SOCKET == _socket)
        return HAPI::LastError(L"new socket");

    char reuseAddr = 1;
    if (SOCKET_ERROR == setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)))
        return HAPI::LastError(L"socket option, SO_REUSEADDR");

    int sendBuffer = HNetPacket::MAX_SIZE;
    if (SOCKET_ERROR == setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, (const char*)&sendBuffer, sizeof(sendBuffer)))
        return HAPI::LastError(L"socket option, SO_SNDBUF");
    
    int recvBuffer = HNetPacket::MAX_SIZE;
    if (SOCKET_ERROR == setsockopt(_socket, SOL_SOCKET, SO_RCVBUF, (const char*)&recvBuffer, sizeof(recvBuffer)))
        return HAPI::LastError(L"socket option, SO_RCVBUF");

    char nodelay = 1;
    if (SOCKET_ERROR == setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay)))
        return HAPI::LastError(L"socket option, TCP_NODELAY");

    linger Linger = {1, 0};
    if (SOCKET_ERROR == setsockopt(_socket, SOL_SOCKET, SO_LINGER, (const char*)&Linger, sizeof(Linger)))
        return HAPI::LastError(L"socket option, SO_LINGER");

    if (AF_INET6 == af)
    {
        int on = 0;
        if (SOCKET_ERROR == setsockopt(_socket, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&on, sizeof(on)))
            return HAPI::LastError(L"socket option, IPV6_V6ONLY");
    }

    return true;
}

void HNetSession::Close()
{
    closesocket(_socket);

    _socket = INVALID_SOCKET;
}

void HNetSession::Cancel()
{
    // CancelIo()는 컴플리션 포트와 연계되어 있는 경우를 제외한다.
    // 그래서 CancelExIo() 를 사용. NULL 을 전달해서 모든 I/O 요청을 취소한다.
    CancelIoEx(*this, NULL);
}

bool HNetSession::SetAddress(const wchar_t* host, USHORT port)
{ 
    if (NULL == host)
        return false;

    return _HostToAddr(host, port);
}

bool HNetSession::SetAddress(ADDRESS* address)
{
    memcpy(&_address, address, sizeof(_address));
    return SetAddress(GetIp(), GetPort());
}

sockaddr* HNetSession::GetAddress()
{ 
    return (sockaddr*)&_address;
}

int HNetSession::GetAddressLenth()
{
    return ((AF_INET == _address.ss_family) ? sizeof(SOCKADDR_IN) : sizeof(SOCKADDR_IN6)) + 16;
}

ADDRESS_FAMILY HNetSession::GetAddressFamily()
{
    return _address.ss_family;
}

const wchar_t* HNetSession::GetAddressFamilyString()
{
    if (AF_INET == GetAddressFamily())
        return L"ipv4";
    else if (AF_INET6 == GetAddressFamily())
        return L"ipv6";
    
    return L"null";
}

const wchar_t* HNetSession::GetIp()
{
	static thread_local wchar_t ip[HNET_HOST_LENTH];
    ip[0] = '\0';
    
    if (AF_INET == _address.ss_family)
    {
        SOCKADDR_IN* addr = (SOCKADDR_IN*)&_address;
        InetNtopW(AF_INET, &addr->sin_addr.s_addr, ip, _countof(ip));
    }
    else
    {
        SOCKADDR_IN6* addr = (SOCKADDR_IN6*)&_address;
        InetNtopW(AF_INET6, &addr->sin6_addr, ip, _countof(ip));
    }
	
	return ip;
}

USHORT HNetSession::GetPort()
{
    USHORT port = 0;
    if (AF_INET == _address.ss_family)
    {
        SOCKADDR_IN* addr = (SOCKADDR_IN*)&_address;
        port = addr->sin_port;
    }
    else
    {
        SOCKADDR_IN6* addr = (SOCKADDR_IN6*)&_address;
        port = addr->sin6_port;
    }

    return ntohs(port);
}

void HNetSession::GetHost(OUT HNetHost& Host)
{
    HAPI::StringCopy(Host._host, GetIp());
    Host._port = GetPort();
}

NetId HNetSession::GetNetId()
{
    return _netId;
}

NetId HNetSession::NewNetId()
{
    static USHORT id = (USHORT)time(NULL);
    
    _netId = ((++id << 16) | (USHORT)_netId);

    return _netId;
}

USHORT HNetSession::Id()
{
    return (_netId & 0x0000FFFF);
}

bool HNetSession::IsValid()
{
    return (INVALID_SOCKET != _socket);
}

bool HNetSession::_AddIoJob(JOB_TYPE type)
{
    if (false == _hnet->IsSingleThread())
        return false;

    HNetIoJob* job = NULL;

    switch (type)
    {
    case JOB_TYPE::ACCEPT:     job = new HNetIoAcceptJob(this);     break;
    case JOB_TYPE::RECONNECT:  job = new HNetIoReconnectJob(this);  break;
    case JOB_TYPE::DISCONNECT: job = new HNetIoDisconnectJob(this); break;
    case JOB_TYPE::RECV:       job = new HNetIoRecvJob(this);       break;
    case JOB_TYPE::SEND:       job = new HNetIoSendJob(this);       break;
    }

    if (NULL == job)
        return false;

    _hnet->_AddIoJob(job);

    return true;
}

bool HNetSession::_HostToAddr(const wchar_t* host, USHORT port)
{
    ADDRINFOW hint;
    ZeroMemory(&hint, sizeof(hint));
    hint.ai_family   = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_protocol = IPPROTO_TCP;

    ADDRINFOW* result = NULL;
    GetAddrInfoW(host, NULL, &hint, &result);

    for (ADDRINFOW* p = result; p != NULL; p = p->ai_next)
    {
        if (AF_INET == p->ai_family)
        {
            memcpy(&_address, p->ai_addr, sizeof(sockaddr_in));
            sockaddr_in* ipv4 = (sockaddr_in*)&_address;
            ipv4->sin_port    = htons(port);
            return true;
        }

        if (AF_INET6 == p->ai_family)
        {
            memcpy(&_address, p->ai_addr, sizeof(sockaddr_in6));
            sockaddr_in6* ipv6 = (sockaddr_in6*)&_address;
            ipv6->sin6_port    = htons(port);
            return true;
        }
    }

    return false;
}

bool HNetSession::_OnListen()
{
    if (false == _hnet->_Associate(this))
        return false;

    Reference();

    return true;
}

bool HNetSession::_OnConnect()
{
    Reference();

    if (_hnet->_Connected(this))
    {
        Recv();
    }

    Release();

    return true;
}

void HNetSession::_OnTryReconnect()
{
    _hnet->_PostIo(HNetIoReconnect::Overlapped());
}

void HNetSession::_OnReconnect()
{
    if (_AddIoJob(JOB_TYPE::RECONNECT))
        return;
    
    Reference();

    if (_hnet->_Reconnected(this))
    {
        Recv();
    }

    Release();
}

void HNetSession::_OnReconnectJob()
{
    Reference();

    if (_hnet->_Reconnected(this))
    {
        Recv();
    }

    Release();
}

void HNetSession::_OnAcceptJob()
{
    _AddIoJob(JOB_TYPE::ACCEPT);
}

void HNetSession::_OnSendJob()
{
    _AddIoJob(JOB_TYPE::SEND);
}

void HNetSession::_OnConnectWs()
{
    if (false == _hnet->_Connected(this))
        return;

    RecvWs();
}

void HNetSession::_OnMessage(const HNetPacket& packet)
{
    _hnet->_Decrypt(packet.Type(), (char*)(&packet + 1), packet.Lenth());
    _hnet->_Message(this, packet);
}

void HNetSession::_OnMessageJob()
{
    _AddIoJob(JOB_TYPE::RECV);
}

void HNetSession::_OnMessage(const HNetWsPacket& packet)
{
    _hnet->_Message(this, packet);
}

void HNetSession::_OnDisconnect()
{
    if (false == IsValid())
        return;

    Reference();

    Cancel();

    Release();
}

void HNetSession::_OnDisconnectJob()
{
    _hnet->_Disconnected(this);
}

void HNetSession::OnRelease()
{
    HNetIoSend::Clear();

    if (_AddIoJob(JOB_TYPE::DISCONNECT))
        return;

    _hnet->_Disconnected(this);
}
