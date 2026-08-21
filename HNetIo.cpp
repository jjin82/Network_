#pragma warning(disable:4100 4706 4996)

#include "HNetSession.h"
#include "HNetIo.h"

Performance g_performance;

#ifdef HNETWORK_FULL_SOURCE
class PerformanceDisplay : public HLIB::Thread
{
public:
    PerformanceDisplay() : HLIB::Thread(L"PerformanceDisplay")
    {
        Run();
    }

    void OnWorker() override
    {
        // 1초 대기.
        Wait(1000);

        // 1초 후 정보 출력.
        HAPI::Print(1, 20, L"▶ recv max bytes: %d ",    g_performance._recvBytesMax);
        HAPI::Print(1, 21, L"▶ recv I/O count: %d",     g_performance._recvIoCount);
        
        HAPI::Print(1, 23, L"▶ send max bytes: %d",     g_performance._sendBytesMax);
        HAPI::Print(1, 24, L"▶ send I/O count: %d",     g_performance._sendIoCount);
    }
};
#endif

bool SocketError(NetId netId, const wchar_t* error, int line = __LINE__);
bool SocketError(const wchar_t* error, int line = __LINE__);

//■=============================================================================================■
//   HNetIo.
//■=============================================================================================■
HNetIo::HNetIo(IO_TYPE type, HNetSession* session)
    : _type(type)
    , _session(session)
{
#ifdef HNETWORK_FULL_SOURCE
    static PerformanceDisplay performanceDisplay;
#endif
}

HNetIo::~HNetIo()
{

}

HNetIo* HNetIo::Overlapped()
{
    OVERLAPPED::Internal     = 0;
    OVERLAPPED::InternalHigh = 0;
    OVERLAPPED::Offset       = 0;
    OVERLAPPED::OffsetHigh   = 0;
    OVERLAPPED::hEvent       = NULL;

    return this;
}

void HNetIo::Completion(DWORD bytes)
{
    switch (_type)
    {
    case IO_TYPE::SEND:      _session->Sended(bytes);     break;
    case IO_TYPE::RECV:      _session->Recved(bytes);     break;
    case IO_TYPE::RECV_WS:   _session->RecvedWs(bytes);   break;
    case IO_TYPE::ACCEPT:    _session->Accepted();        break;
    case IO_TYPE::ACCEPT_WS: _session->AcceptedWs(bytes); break;
    case IO_TYPE::RECONNECT: _session->Reconnect();       return;
    }

    _session->Release();
}

void HNetIo::CompletionJob(DWORD bytes)
{
    switch (_type)
    {
    case IO_TYPE::SEND:      _session->Sended(bytes);     break;
    case IO_TYPE::RECV:      _session->RecvedJob(bytes);  break;
    case IO_TYPE::RECV_WS:   _session->RecvedWs(bytes);   break;
    case IO_TYPE::ACCEPT:    _session->AcceptedJob();     break;
    case IO_TYPE::ACCEPT_WS: _session->AcceptedWs(bytes); break;
    case IO_TYPE::RECONNECT: _session->Reconnect();       return;
    }

    _session->Release();
}

//■=============================================================================================■
//   HNetIoListen.
//■=============================================================================================■
HNetIoListen::HNetIoListen(HNetSession* session)
    : HNetIo(IO_TYPE::LISTEN, session)
{

}

bool HNetIoListen::Listen(wchar_t* ip, USHORT port)
{
    if (false == _session->SetAddress(ip, port))
        return false;

    if (false == _session->Open())
        return false;

    if (SOCKET_ERROR == bind(*_session, _session->GetAddress(), _session->GetAddressLenth()))
    {
        SocketError(L"bind");
        return false;
    }

    // listen() 에서 자동적으로 accept를 받지 못하도록 하는 옵션.(bind 처리를 마친 소켓)
    const char opt = 1; 
    if (SOCKET_ERROR == setsockopt(*_session, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, &opt, sizeof(opt)))
    {
        SocketError(L"setsockopt(SO_CONDITIONAL_ACCEPT)");
        return false;
    }

    if (SOCKET_ERROR == listen(*_session, 1))
    {
        SocketError(L"listen");
        return false;
    }

    return _session->_OnListen();
}

//■=============================================================================================■
//   HNetIoAccept.
//■=============================================================================================■
HNetIoAccept::HNetIoAccept(HNetSession* session)
    : HNetIo(IO_TYPE::ACCEPT, session)
{
    ZeroMemory(_buf, sizeof(_buf));
}

bool HNetIoAccept::Accept(HNetSession* listen)
{
    if (false == _session->Open(listen->GetAddressFamily()))
    {
        _session->Disconnect(L"Failed to open the session.");
        return false;
    }

    _session->Reference();

    DWORD outBytes = 0;
    if (FALSE == AcceptEx(*listen, *_session, _buf, 0, _session->GetAddressLenth(), _session->GetAddressLenth(), &outBytes, HNetIo::Overlapped()))
    {
        if (SocketError(_session->GetNetId(), L"AcceptEx"))
        {
            _session->Disconnect(L"Failed to register the session for accepting.");
            _session->Release();
            return false;
        }
    }

    return true;
}

void HNetIoAccept::Accepted()
{
    ADDRESS* localAddr  = NULL;
    ADDRESS* remoteAddr = NULL;
    int      localLen   = 0;
    int      remoteLen  = 0;

    GetAcceptExSockaddrs(_buf, 0, _session->GetAddressLenth(), _session->GetAddressLenth(), (sockaddr**)&localAddr, &localLen, (sockaddr**)&remoteAddr, &remoteLen);

    if (false == _session->SetAddress(remoteAddr))
    {
        _session->Disconnect(L"Failed to obtain session address information.");
        return;
    }

    _session->_OnConnect();
}

void HNetIoAccept::AcceptedJob()
{
    _session->_OnAcceptJob();
}

void HNetIoAccept::operator >> (HNetIoAccept* ioAccept)
{
    ioAccept->_session = _session;
    memcpy(ioAccept->_buf, _buf, sizeof(ioAccept->_buf));
}

//■=============================================================================================■
//   HNetIoConnect.
//■=============================================================================================■
HNetIoConnect::HNetIoConnect(HNetSession* session)
    : HNetIo(IO_TYPE::CONNECT, session)
{

}

bool HNetIoConnect::Connect(const wchar_t* host, USHORT port)
{
    if (false == _session->SetAddress(host, port))
        return false;

    if (false == _session->Open())
        return false;

    if (SOCKET_ERROR == connect(*_session, _session->GetAddress(), _session->GetAddressLenth()))
    {
        SocketError(L"connect");
        _session->Close();
        return false;
    }

    return _session->_OnConnect();
}

void HNetIoConnect::operator >> (HNetIoConnect* ioConnect)
{
    ioConnect->_session = _session;
}


//■=============================================================================================■
//   HNetIoReconnect.
//■=============================================================================================■
HNetIoReconnect::HNetIoReconnect(HNetSession* session)
    : HNetIo(IO_TYPE::RECONNECT, session)
{

}

void HNetIoReconnect::TryReconnect()
{
    _session->_OnTryReconnect();
}

void HNetIoReconnect::Reconnect()
{
    // n초 정도는 간격으로 두고 시도.
    HNET::API::Sleep(500);

    if (false == _session->Open())
        return;

    if (SOCKET_ERROR == connect(*_session, _session->GetAddress(), sizeof(SOCKADDR_IN)))
    {
        SocketError(L"reconnect");
        TryReconnect();
        return;
    }

    _session->_OnReconnect();
}

void HNetIoReconnect::ReconnectJob()
{
    _session->_OnReconnectJob();
}

void HNetIoReconnect::operator >> (HNetIoReconnect* ioReconnect)
{
    ioReconnect->_session = _session;
}


//■=============================================================================================■
//   HNetIoDisconnect.
//■=============================================================================================■
HNetIoDisconnect::HNetIoDisconnect(HNetSession* session)
    : HNetIo(IO_TYPE::DISCONNECT, session)
    , _disconnect(NULL)
{

}

bool HNetIoDisconnect::Initialize()
{
    _disconnect = NULL;

    return true;
}

void HNetIoDisconnect::Disconnect(const wchar_t* reason)
{
    InterlockedCompareExchangePointer((PVOID*)&_disconnect, const_cast<wchar_t*>(reason), NULL);

    _session->_OnDisconnect();
}

void HNetIoDisconnect::DisconnectJob()
{
    _session->_OnDisconnectJob();
}

bool HNetIoDisconnect::IsDisconnect()
{
    return (NULL != _disconnect);
}

const wchar_t* HNetIoDisconnect::DisconnectReason()
{
    return _disconnect;
}

void HNetIoDisconnect::operator >> (HNetIoDisconnect* ioDisconnect)
{
    ioDisconnect->_session = _session;
}

//■=============================================================================================■
//   HNetIoRecv.
//■=============================================================================================■
HNetIoRecv::HNetIoRecv(HNetSession* session)
    : HNetIo(IO_TYPE::RECV, session)
    , _buf(NULL)
    , _size(0)
    , _begin(0)
    , _end(0)
{
}

HNetIoRecv::~HNetIoRecv()
{
    HAPI::Dealloc(_buf);
}

bool HNetIoRecv::Create()
{
    _size = HNetPacket::MAX_SIZE;

    return true;
}

bool HNetIoRecv::Initialize()
{
    // 소켓을 실재 사용할때 메모리를 할당한다.
    if (NULL == _buf)
    {
        _buf = (char*)HAPI::Alloc(_size);
        if (NULL == _buf) return false;
    }

    _begin = 0;
    _end   = 0;

    return true;
}

bool HNetIoRecv::Flip()
{
    _buf = (char*)HAPI::Alloc(_size);
    if (NULL == _buf) return false;

    return true;
}

bool HNetIoRecv::IsValid(DWORD size)
{
	return (sizeof(HNetPacket) <= size);
}

WSABUF* HNetIoRecv::PrepareBuffer()
{
    static thread_local WSABUF t_wsaBuf;

    // 버퍼가 비어 있다.
    if (_begin == _end)
    {
        _begin       = 0;
        _end         = 0;
        t_wsaBuf.len = _size;
    }
    else if (_begin)
    {
        // 완료처리가 안된 버퍼의 앞으로 복사.
        _end = (_end - _begin);
        memcpy(_buf, &_buf[_begin], _end);
        _begin = 0;

        t_wsaBuf.len = (_size - _end);
    }

    t_wsaBuf.buf = &_buf[_end];

    return &t_wsaBuf;
}

bool HNetIoRecv::Completed(DWORD bytes)
{
    // 완료처리가 안된 버퍼의 크기.
    DWORD incomplete = (_end - _begin) + bytes;

    while (bytes)
    {
        if (false == IsValid(incomplete))
        {
            _end += bytes;
            break;
        }
        
        HNetPacket* packet = (HNetPacket*)(&_buf[_begin]);
        if (false == packet->Check()) return false;

        // 미완성 패킷.
        if (packet->Size() > incomplete)
        {
            _end += bytes;
            break;
        }
        else
        {
            _session->_OnMessage(*packet);

            bytes      -= packet->Size() - (_end - _begin);
            incomplete -= packet->Size();

            _begin     += packet->Size();
            _end        = _begin;
        }
    }

    return true;
}

bool HNetIoRecv::CompletedJob(DWORD bytes)
{
    // 완료처리가 안된 버퍼의 크기.
    DWORD incomplete = (_end - _begin) + bytes;

    while (bytes)
    {
        if (false == IsValid(incomplete))
        {
            _end += bytes;
            break;
        }

        HNetPacket* packet = (HNetPacket*)(&_buf[_begin]);
        if (false == packet->Check())
            return false;

        // 미완성 패킷.
        if (packet->Size() > incomplete)
        {
            _end += bytes;
            break;
        }
        else
        {
            bytes      -= packet->Size() - (_end - _begin);
            incomplete -= packet->Size();

            _begin     += packet->Size();
            _end        = _begin;
        }
    }

    _session->_OnMessageJob();

    return true;
}

void HNetIoRecv::Recv()
{
    if (_session->IsDisconnect())
        return;

    _session->Reference();

    DWORD outBytes   = 0;
    DWORD inOutFlags = 0;
    if (SOCKET_ERROR == WSARecv(*_session, PrepareBuffer(), 1, &outBytes, &inOutFlags, Overlapped(), NULL))
    {
        if (WSA_IO_PENDING == GetLastError())
            return;
        
        _session->Disconnect(L"There was an error during the receive process.");
        _session->Release();
    }
}

void HNetIoRecv::Recved(DWORD bytes)
{
    g_performance.RecvIo(bytes);

    if (0 == bytes)
    {
        _session->Disconnect(L"The client has disconnected.");
        return;
    }

    if (false == Completed(bytes))
    {
        _session->Disconnect(L"There is an error in the received packet information.");
        return;
    }

    Recv();
}

void HNetIoRecv::RecvedJob(DWORD bytes)
{
    if (0 == bytes)
    {
        _session->Disconnect(L"The client has disconnected.(job)");
        return;
    }

    if (false == CompletedJob(bytes))
    {
        _session->Disconnect(L"There is an error in the received packet information.(job)");
        return;
    }

    Recv();
}

DWORD HNetIoRecv::operator >> (HNetIoRecv* ioRecv)
{
    // 복사.
    ioRecv->_session = _session;
    ioRecv->_buf    = _buf;
    ioRecv->_size   = _size;
    ioRecv->_begin  = 0;
    ioRecv->_end    = 0;

    // 버퍼 교체.
    Flip();

    // 완성되지 않은 패킷 복사.
    if (_begin != _end)
        memcpy(&_buf[_begin], &ioRecv->_buf[_begin], (_end - _begin));
    
    return _begin;
}

//■=============================================================================================■
//   Gathering.
//■=============================================================================================■
class Gathering
{
    friend HNetIoSend;
public:
    Gathering()
        : _lenth(0)
        , _count(0)
        , _next(NULL)
    {
    }

    void Release()
    {
        _lenth = 0;
        _count = 0;
    }

    bool Append(char* buf, int lenth)
    {
        if (HNetPacket::MAX_SIZE <= (lenth + _lenth))
            return false;

        memcpy(&_buf[_lenth], buf, lenth);

        _lenth += lenth;
        _count += 1;

        return true;
    }

private:
    char       _buf[HNetPacket::MAX_SIZE];
    int        _lenth;
    int        _count;
    Gathering* _next;
};

//■=============================================================================================■
//   HNetIoSend.
//■=============================================================================================■
HNetIoSend::HNetIoSend(HNetSession* session)
    : HNetIo(IO_TYPE::SEND, session)
    , _head(NULL)
    , _tail(NULL)
    , _pending(false)
{

}

HNetIoSend::~HNetIoSend()
{

}

bool HNetIoSend::Initialize()
{
    if (NULL == _head)
    {
        _head        = new Gathering;
        _head->_next = _head;
        _tail        = _head;
    }

    _pending = false;

    return true;
}

void HNetIoSend::Clear()
{
    Gathering* cur = _head; 
    do
    {
        cur->Release();
        cur = cur->_next;
    }
    while (cur != _head);

    _head = _tail;
}

void HNetIoSend::NextBuffer()
{
    if (_head == _tail->_next)
    {
        _tail->_next        = new Gathering;
        _tail->_next->_next = _head;
    }

    _tail = _tail->_next;
}

void HNetIoSend::Send(WsaSend* wsaSend)
{
    _lock.LockW();
    {
        if (NULL == _tail)
            return;

        while (false == _tail->Append(wsaSend->buf, wsaSend->len))
        {
            NextBuffer();
        }

        if (_pending)
        {
            _lock.UnlockW();
            return;
        }

        _pending = true;

        NextBuffer();
    }
    _lock.UnlockW();

    _Send();
}

void HNetIoSend::Sended(DWORD bytes)
{
    g_performance.SendIo(bytes);

    _head->Release();

    _lock.LockW();
    {
        _head = _head->_next;
        if (0 == _head->_lenth)
        {
            _pending = false;
            _lock.UnlockW();
            return;
        }
        else if (_tail == _head)
        {
            _tail = _tail->_next;
        }
    }
    _lock.UnlockW();

    _Send();
}

void HNetIoSend::SendJob(WsaSend* wsaSend)
{
    _lock.LockW();
    {
        if (NULL == _tail)
            return;

        while (false == _tail->Append(wsaSend->buf, wsaSend->len))
        {
            NextBuffer();
        }

        if (_pending)
        {
            _lock.UnlockW();
            return;
        }

        _pending = true;
     
        _session->_OnSendJob();
    }
    _lock.UnlockW();
}

void HNetIoSend::SendedJob()
{
    _lock.LockW();
    {
        if (0 == _head->_count)
        {
            _pending = false;
            _lock.UnlockW();
            return;
        }

        if (_head == _tail)
        {
            _tail = _tail->_next;
        }
    }
    _lock.UnlockW();

    _Send();
}

void HNetIoSend::operator >> (HNetIoSend* ioSend)
{
    ioSend->_session = _session;
}

void HNetIoSend::_Send()
{
    _session->Reference();

    WSABUF wsaBuf;
    wsaBuf.buf = _head->_buf;
    wsaBuf.len = _head->_lenth;

    DWORD outBytes = 0;
    if (SOCKET_ERROR == WSASend(*_session, &wsaBuf, 1, &outBytes, 0, Overlapped(), NULL))
    {
        if (WSA_IO_PENDING == GetLastError())
            return;

        _session->Disconnect(L"Failed to send the packet.");
        _session->Release();
    }
}

//■=============================================================================================■
//   HNetIoWsAccept.
//■=============================================================================================■
class SHA1
{
public:
    SHA1();
    const char* ToBase64(const char* string);

private:
    const char* _ToBinary();
    bool _Input(const char* message_array);
    void _ProcessMessageBlock();
    void _PadMessage();
    inline unsigned _CircularShift(int bits, unsigned word);

private:
    unsigned H[5];

    unsigned _lengthLow;
    unsigned _lengthHigh;

    unsigned char _block[64];
    int _blockIndex;

    char _encoded[1024];
};

HNetIoWsAccept::HNetIoWsAccept(HNetSession* session)
    : HNetIo(IO_TYPE::ACCEPT_WS, session)
{

}

bool HNetIoWsAccept::AcceptWs(HNetSession* listen)
{
    _session->Reference();

    if (false == _session->Open(listen->GetAddressFamily()))
    {
        _session->Disconnect(L"Failed to open the session.(ws)");
        _session->Release();
        return false;
    }

    DWORD outBytes = 0;
    if (FALSE == AcceptEx(*listen, *_session, _buf, RECV_SIZE, _session->GetAddressLenth(), _session->GetAddressLenth(), &outBytes, Overlapped()))
    {
        if (SocketError(_session->GetNetId(), L"AcceptEx"))
        {
            _session->Disconnect(L"Failed to register the session for accepting.");
            _session->Release();
            return false;
        }
    }

    return true;
}

void HNetIoWsAccept::AcceptedWs(DWORD bytes)
{
    ADDRESS* localAddr  = NULL;
    ADDRESS* remoteAddr = NULL;
    int      localLen   = 0;
    int      remoteLen  = 0;

    GetAcceptExSockaddrs(_buf, RECV_SIZE, _session->GetAddressLenth(), _session->GetAddressLenth(), (sockaddr**)&localAddr, &localLen, (sockaddr**)&remoteAddr, &remoteLen);

    if (false == _session->SetAddress(remoteAddr))
    {
        _session->Disconnect(L"Failed to obtain session address information.(ws)");
        return;
    }

    // 웹은 handshake 이 성공해야 송, 수신이 되므로 접속 성공 관한 처리를 하고 수신 대기 상태로 만들자.
    _session->_OnConnectWs();

    // handshake.
    if (false == _Handshake())
    {
        _session->Disconnect(L"Failed during session handshake processing.");
        return;
    }

    // ping.
    _Ping();
}

bool HNetIoWsAccept::_Handshake()
{
    // GUID.
    static const char guid[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    // 키값 구분자.
    static const char valueDelimiter[] = " ";

    // new memory.
    WsaSend* wsaSend = (WsaSend*)HAPI::Alloc(sizeof(WsaSend) + BUFF_SIZE);
    if (NULL == wsaSend)
        return false;

    // data.
    char* response = (char*)(wsaSend + 1);
    ZeroMemory(response, BUFF_SIZE);
    strncat(response, "HTTP/1.1 101 Switching Protocols\r\n", BUFF_SIZE);
    strncat(response, "Upgrade: WebSocket\r\n", BUFF_SIZE);
    strncat(response, "Connection: Upgrade\r\n", BUFF_SIZE);

    HLIB::StrParser parser;
    parser.Parsing(_buf, "\r\n");
    const char* key = parser.KeyValue("Sec-WebSocket-Key:", valueDelimiter);
    if (NULL == key)
    {
        LOG_CRITICAL_SYSTEM(L"Sec-WebSocket-Key error.");
        return false;
    }
    
    char totalKey[BUFF_SIZE];
    ZeroMemory(totalKey, sizeof(totalKey));
    sprintf_s(totalKey, "%s%s", key, guid);

    SHA1 sha1;
    const char* accept = sha1.ToBase64(totalKey);

    strncat(response, "Sec-WebSocket-Accept: ", BUFF_SIZE);
    strncat(response, accept, BUFF_SIZE);
    strncat(response, "\r\n", BUFF_SIZE);

    const char* protocol = parser.KeyValue("Sec-WebSocket-Protocol:", valueDelimiter);
    if (protocol)
    {
        strncat(response, "Sec-WebSocket-Protocol: ", BUFF_SIZE);
        strncat(response, protocol, BUFF_SIZE);
        strncat(response, "\r\n", BUFF_SIZE);
    }

    // end.
    strncat(response, "\r\n", BUFF_SIZE);

    // set send buffer.
    wsaSend->buf       = response;
    wsaSend->len       = (ULONG)strnlen(response, BUFF_SIZE);
    wsaSend->_refCount = 0;

    // send.
    _session->Send(wsaSend);

    return true;
}

void HNetIoWsAccept::_Ping()
{
    const char data[] = "H";
    const char lenth = (char)sizeof(data);

    // set send buffer.
    WsaSend* wsaSend  = (WsaSend*)HAPI::Alloc(sizeof(WsaSend) + lenth);
    wsaSend->len       = sizeof(HNetWsPacket::Header) + lenth;
    wsaSend->_refCount = 0;

    // set ping.
    wsaSend->buf    = (char*)(wsaSend + 1);
    wsaSend->buf[0] = (unsigned char)HNetWsPacket::Header::PING_TYPE;
    wsaSend->buf[1] = lenth;
    memcpy(&wsaSend->buf[2], data, lenth);

    // send.
    _session->Send(wsaSend);
}

//■=============================================================================================■
//   HNetIoWsRecv.
//■=============================================================================================■
HNetIoWsRecv::HNetIoWsRecv(HNetSession* session)
    : HNetIo(IO_TYPE::RECV_WS, session)
    , _buf(NULL)
    , _size(0)
    , _begin(0)
    , _end(0)
{
}

HNetIoWsRecv::~HNetIoWsRecv()
{

}

bool HNetIoWsRecv::Create()
{
    _size = HNetPacket::MAX_SIZE;

    return true;
}

bool HNetIoWsRecv::Initialize()
{
    if (NULL == _buf)
    {
        _buf = (char*)HAPI::Alloc(_size);
        if (NULL == _buf)
            return false;
    }

    _begin = 0;
    _end   = 0;

    return true;
}

bool HNetIoWsRecv::IsValid(DWORD size)
{
	return (sizeof(HNetWsPacket::Header) <= size);
}

WSABUF* HNetIoWsRecv::GetWsaBuf()
{
    static thread_local WSABUF t_wsaBuf;

    // 버퍼가 비어 있다.
    if (_begin == _end)
    {
        _begin     = 0;
        _end       = 0;
        t_wsaBuf.len = _size;
    }
    else if (_begin)
    {
        // 완료처리가 안된 버퍼의 앞으로 복사.
        DWORD incomplete = (_end - _begin);
        memcpy(_buf, &_buf[_begin], incomplete);

        _begin     = 0;
        _end       = incomplete;
        t_wsaBuf.len = (_size - _end);
    }

    t_wsaBuf.buf = &_buf[_end];

    return &t_wsaBuf;
}

bool HNetIoWsRecv::Completed(DWORD bytes)
{
    // 완료처리가 안된 버퍼의 크기.
    DWORD incomplete = (_end - _begin) + bytes;

    while (bytes)
    {
        if (false == IsValid(incomplete))
        {
            _end += bytes;
            return true;
        }
        
        HNetWsPacket::Header* header = (HNetWsPacket::Header*)(&_buf[_begin]);
        if (false == header->Check())
            return false;

        // 미완성 패킷.
        if (header->Size() > incomplete)
        {
            _end += bytes;
            return true;
        }
        else
        {
            USHORT size = _Preprocess(header);
            if (0 == size)
                return false;

            bytes      -= size - (_end - _begin);
            incomplete -= size;

            _begin     += size;
            _end        = _begin;
        }
    }

    return true;
}

void HNetIoWsRecv::RecvWs()
{
    _session->Reference();

    DWORD outBytes   = 0;
    DWORD inOutFlags = 0;
    if (SOCKET_ERROR == WSARecv(*_session, GetWsaBuf(), 1, &outBytes, &inOutFlags, Overlapped(), NULL))
    {
        if (SocketError(_session->GetNetId(), L"recv"))
        {
            _session->Disconnect(L"There was an error during the receive process.(ws)");
            _session->Release();
        }
    }
}

void HNetIoWsRecv::RecvedWs(DWORD bytes)
{
    if (0 == bytes)
    {
        _session->Disconnect(L"The client has disconnected.(ws)");
        return;
    }

    if (false == Completed(bytes))
    {
        _session->Disconnect(L"There is an error in the received packet information.(ws)");
        return;
    }

    RecvWs();
}

USHORT HNetIoWsRecv::_Preprocess(HNetWsPacket::Header* header)
{
    switch (header->Opcode())
    {
    case HNetWsPacket::Header::OPCODE_DISCONNECT: return 0;
    case HNetWsPacket::Header::OPCODE_PING:       _Pong();
    case HNetWsPacket::Header::OPCODE_PONG:       break;
    default:
        {
            HNetWsPacket* packet = header->ToPacket();
            if (NULL == packet)
                return 0;

            _session->_OnMessage(*packet);
        }
    }

    return header->Size();
}

void HNetIoWsRecv::_Pong()
{
    // set send buffer.
    WsaSend* wsaSend  = (WsaSend*)HAPI::Alloc(sizeof(WsaSend) + 32);
    wsaSend->buf       = NULL;
    wsaSend->len       = 0;
    wsaSend->_refCount = 0;

    // send.
    _session->Send(wsaSend);
}

//■=============================================================================================■
//   HNetIoJob.
//■=============================================================================================■
void* HNetIoJob::operator new(size_t size)
{
    return HAPI::Alloc(size);
}

void HNetIoJob::operator delete(void* p)
{
    HAPI::Dealloc(p);
}

//■=============================================================================================■
//   HNetIoAcceptJob.
//■=============================================================================================■
HNetIoAcceptJob::HNetIoAcceptJob(HNetIoAccept* ioAccept)
    : HNetIoAccept(NULL)
{
    *ioAccept >> this;

    _session->Reference();
}

void HNetIoAcceptJob::Excute()
{
    Accepted();

    _session->Release();
}

//■=============================================================================================■
//   HNetIoReconnectJob.
//■=============================================================================================■
HNetIoReconnectJob::HNetIoReconnectJob(HNetIoReconnect* ioConnect)
    : HNetIoReconnect(NULL)
{
    *ioConnect >> this;
}

void HNetIoReconnectJob::Excute()
{
    ReconnectJob();
}

//■=============================================================================================■
//   HNetIoDisconnectJob.
//■=============================================================================================■
HNetIoDisconnectJob::HNetIoDisconnectJob(HNetIoDisconnect* io)
    : HNetIoDisconnect(NULL)
{
    *io >> this;
}

void HNetIoDisconnectJob::Excute()
{
    DisconnectJob();
}

//■=============================================================================================■
//   HNetIoRecvJob.
//■=============================================================================================■
HNetIoRecvJob::HNetIoRecvJob(HNetIoRecv* ioRecv)
    : HNetIoRecv(NULL)
{
    _byte = (*ioRecv >> this);

    _session->Reference();
}

void HNetIoRecvJob::Excute()
{
    Completed(_byte);

    _session->Release();
}

//■=============================================================================================■
//   HNetIoSendJob.
//■=============================================================================================■
HNetIoSendJob::HNetIoSendJob(HNetIoSend* io)
    : HNetIoSend(NULL) 
{
    *io >> this;

    _session->Reference();
}

void HNetIoSendJob::Excute()
{
    _session->SendedJob();

    _session->Release();
}

//■=============================================================================================■
//   SocketError.
//■=============================================================================================■
bool SocketError(NetId netId, const wchar_t* error, int line)
{
    DWORD errorCode = GetLastError();
    if (WSA_IO_PENDING == errorCode)
        return false;

    static thread_local wchar_t string[1024];
    FormatMessageW((FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS), NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), string, _countof(string), NULL);

    // errorText 에서 마지막 '\r\n' 문자 제거.
    string[max(0, wcslen(string) - 2)] = '\0';

    LOG_LAST_ERROR_SYSTEM(L"netId=\"%u\", comment=\"%s\", code(%d)=\"%s\", line=\"%d\"", netId, error, errorCode, string, line);

    return true;
}

bool SocketError(const wchar_t* error, int line)
{
    return SocketError(0, error, line);
}


//■=============================================================================================■
//   SHA1. (web handshake)
//■=============================================================================================■
#pragma warning(disable:4244)
SHA1::SHA1()
{
    H[0] = 0x67452301;
    H[1] = 0xEFCDAB89;
    H[2] = 0x98BADCFE;
    H[3] = 0x10325476;
    H[4] = 0xC3D2E1F0;

    _lengthLow  = 0;
    _lengthHigh = 0;

    _block[0]   = '\0';
    _blockIndex = 0;

    _encoded[0] = '\0';
}

const char* SHA1::ToBase64(const char* key)
{
    static const char basis_64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    if (false == _Input(key))
        return "";

    const char* string = _ToBinary();
    const int   len = 20;
    char*       p   = _encoded;
    int         i   = 0;

    for (i = 0; i < len - 2; i += 3)
    {
        *p++ = basis_64[(string[i] >> 2) & 0x3F];
        *p++ = basis_64[((string[i]      & 0x3) << 4) | ((int)(string[i + 1] & 0xF0) >> 4)];
        *p++ = basis_64[((string[i + 1]  & 0xF) << 2) | ((int)(string[i + 2] & 0xC0) >> 6)];
        *p++ = basis_64[string[i + 2]    & 0x3F];
    }

    if (i < len)
    {
        *p++ = basis_64[(string[i] >> 2) & 0x3F];
        if (i == (len - 1))
        {
            *p++ = basis_64[((string[i] & 0x3) << 4)];
            *p++ = '=';
        }
        else
        {
            *p++ = basis_64[((string[i]     & 0x3) << 4) | ((int)(string[i + 1] & 0xF0) >> 4)];
            *p++ = basis_64[((string[i + 1] & 0xF) << 2)];
        }
        *p++ = '=';
    }

    *p++ = '\0';

    return _encoded;
}

const char* SHA1::_ToBinary()
{
    static thread_local char binary[20];
    for (int i = 0; i < 5; i++)
    {
        binary[(i * 4) + 0] = (H[i] & 0xff000000) >> 24;
        binary[(i * 4) + 1] = (H[i] & 0x00ff0000) >> 16;
        binary[(i * 4) + 2] = (H[i] & 0x0000ff00) >> 8;
        binary[(i * 4) + 3] = (H[i] & 0x000000ff);
    }

    return binary;
}

void SHA1::_ProcessMessageBlock()
{
    const unsigned K[] = { 0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6 };
    int            t;
    unsigned       temp;
    unsigned       W[80];
    unsigned       A, B, C, D, E;

    for (t = 0; t < 16; t++)
    {
        W[t] = ((unsigned)_block[t * 4]) << 24;
        W[t] |= ((unsigned)_block[t * 4 + 1]) << 16;
        W[t] |= ((unsigned)_block[t * 4 + 2]) << 8;
        W[t] |= ((unsigned)_block[t * 4 + 3]);
    }

    for (t = 16; t < 80; t++)
    {
        W[t] = _CircularShift(1, W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16]);
    }

    A = H[0];
    B = H[1];
    C = H[2];
    D = H[3];
    E = H[4];

    for (t = 0; t < 20; t++)
    {
        temp = _CircularShift(5, A) + ((B & C) | ((~B) & D)) + E + W[t] + K[0];
        temp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = _CircularShift(30, B);
        B = A;
        A = temp;
    }

    for (t = 20; t < 40; t++)
    {
        temp = _CircularShift(5, A) + (B ^ C ^ D) + E + W[t] + K[1];
        temp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = _CircularShift(30, B);
        B = A;
        A = temp;
    }

    for (t = 40; t < 60; t++)
    {
        temp = _CircularShift(5, A) + ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
        temp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = _CircularShift(30, B);
        B = A;
        A = temp;
    }

    for (t = 60; t < 80; t++)
    {
        temp = _CircularShift(5, A) + (B ^ C ^ D) + E + W[t] + K[3];
        temp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = _CircularShift(30, B);
        B = A;
        A = temp;
    }

    H[0] = (H[0] + A) & 0xFFFFFFFF;
    H[1] = (H[1] + B) & 0xFFFFFFFF;
    H[2] = (H[2] + C) & 0xFFFFFFFF;
    H[3] = (H[3] + D) & 0xFFFFFFFF;
    H[4] = (H[4] + E) & 0xFFFFFFFF;

    _blockIndex = 0;
}

void SHA1::_PadMessage()
{
    if (_blockIndex > 55)
    {
        _block[_blockIndex++] = 0x80;
        while (_blockIndex < 64)
        {
            _block[_blockIndex++] = 0;
        }

        _ProcessMessageBlock();

        while (_blockIndex < 56)
        {
            _block[_blockIndex++] = 0;
        }
    }
    else
    {
        _block[_blockIndex++] = 0x80;
        while (_blockIndex < 56)
        {
            _block[_blockIndex++] = 0;
        }

    }

    _block[56] = (_lengthHigh >> 24) & 0xFF;
    _block[57] = (_lengthHigh >> 16) & 0xFF;
    _block[58] = (_lengthHigh >> 8)  & 0xFF;
    _block[59] = (_lengthHigh)       & 0xFF;
    _block[60] = (_lengthLow >> 24)  & 0xFF;
    _block[61] = (_lengthLow >> 16)  & 0xFF;
    _block[62] = (_lengthLow >> 8)   & 0xFF;
    _block[63] = (_lengthLow)& 0xFF;

    _ProcessMessageBlock();
}

unsigned SHA1::_CircularShift(int bits, unsigned word)
{
    return ((word << bits) & 0xFFFFFFFF) | ((word & 0xFFFFFFFF) >> (32 - bits));
}

bool SHA1::_Input(const char* message_array)
{
    size_t length = strlen(message_array);
    if (!length || 64 < length)
        return false;

    while (length--)
    {
        _block[_blockIndex++] = (*message_array & 0xFF);

        _lengthLow += 8;
        _lengthLow &= 0xFFFFFFFF;

        if (_lengthLow == 0)
        {
            _lengthHigh++;
            _lengthHigh &= 0xFFFFFFFF;

            if (_lengthHigh == 0)
                return false;
        }

        if (_blockIndex == 64)
        {
            _ProcessMessageBlock();
        }

        message_array++;
    }

    _PadMessage();

    return true;
}