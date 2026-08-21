#pragma once

#include "HNetCommon.h"
#include "HNetPacket.h"

class HNetSession;
class Gathering;

enum class IO_TYPE : int
{ 
      DISCONNECT
    , LISTEN
	, ACCEPT
    , CONNECT
    , RECONNECT
    , RECV
    , SEND
    , ACCEPT_WS
    , RECV_WS
};

//бс=============================================================================================бс
//   HNetIo.
//бс=============================================================================================бс
class HNetIo : public OVERLAPPED
{
public:
	HNetIo(IO_TYPE type, HNetSession* session);
	~HNetIo();

    HNetIo* Overlapped();
    void Completion(DWORD bytes);
    void CompletionJob(DWORD bytes);

protected:
    IO_TYPE      _type;
    HNetSession* _session;
};

//бс=============================================================================================бс
//   HNetIoListen.
//бс=============================================================================================бс
class HNetIoListen : public HNetIo
{
public:
    HNetIoListen(HNetSession* session);

    bool Listen(wchar_t* ip, USHORT port);
};

//бс=============================================================================================бс
//   HNetIoAccept.
//бс=============================================================================================бс
class HNetIoAccept : public HNetIo
{
    enum { BUFF_SIZE = (sizeof(SOCKADDR_IN) + 16) + (sizeof(SOCKADDR_IN) + 16) };

public:
    HNetIoAccept(HNetSession* session);

    bool Accept(HNetSession* pListen);
    void Accepted();
    void AcceptedJob();

    void operator >> (HNetIoAccept* ioAccept);

private:
    char _buf[BUFF_SIZE];
};

//бс=============================================================================================бс
//   HNetIoConnect.
//бс=============================================================================================бс
class HNetIoConnect : public HNetIo
{
public:
    HNetIoConnect(HNetSession* session);

    bool Connect(const wchar_t* ip, USHORT port);
    
    void operator >> (HNetIoConnect* ioConnect);
};

//бс=============================================================================================бс
//   HNetIoConnect.
//бс=============================================================================================бс
class HNetIoReconnect : public HNetIo
{
public:
    HNetIoReconnect(HNetSession* session);

    void TryReconnect();
    void Reconnect();
    void ReconnectJob();

    void operator >> (HNetIoReconnect* ioConnect);
};

//бс=============================================================================================бс
//   HNetIoDisconnect.
//бс=============================================================================================бс
class HNetIoDisconnect : public HNetIo
{
public:
    HNetIoDisconnect(HNetSession* session);

    bool Initialize();
    void Disconnect(const wchar_t* reason = L"normal connection termination");
    void DisconnectJob();

    bool IsDisconnect();
    const wchar_t* DisconnectReason();

    void operator >> (HNetIoDisconnect* ioDisconnect);

private:
    wchar_t* _disconnect;
};

//бс=============================================================================================бс
//   HNetIoRecv.
//бс=============================================================================================бс
class HNetIoRecv : public HNetIo
{
public:
    HNetIoRecv(HNetSession* session);
    ~HNetIoRecv();

    bool Create();
    bool Initialize();
    bool Flip();
    bool IsValid(DWORD size);

    WSABUF* PrepareBuffer();
    bool Completed(DWORD bytes);
    bool CompletedJob(DWORD bytes);

    void Recv();
    void Recved(DWORD bytes);
    void RecvedJob(DWORD bytes);

    DWORD operator >> (HNetIoRecv* ioRecv);

private:
    char* _buf;
    DWORD _size;
    DWORD _begin;
    DWORD _end;
};

//бс=============================================================================================бс
//   HNetIoSend.
//бс=============================================================================================бс
class HNetIoSend : public HNetIo
{
public:
    HNetIoSend(HNetSession* session);
    ~HNetIoSend();

    bool Initialize();
    void Clear();

    void NextBuffer();

    void Send(WsaSend* wsaSend);
    void Sended(DWORD bytes);
    
    void SendJob(WsaSend* wsaSend);
    void SendedJob();

    void operator >> (HNetIoSend* ioSend);

private:
    void _Send();

private:
    HLIB::RWLock _lock;
    bool         _pending;
    Gathering*   _head;
    Gathering*   _tail;
};

//бс=============================================================================================бс
//   HNetIoWsAccept.
//бс=============================================================================================бс
class HNetIoWsAccept : public HNetIo
{
    enum 
    { 
        BUFF_SIZE = 1024, 
        RECV_SIZE = (BUFF_SIZE - (sizeof(SOCKADDR_IN) + 16) - (sizeof(SOCKADDR_IN) + 16)) 
    };

public:
    HNetIoWsAccept(HNetSession* session);

    bool AcceptWs(HNetSession* listen);
    void AcceptedWs(DWORD bytes);

private:
    bool _Handshake();
    void _Ping();

private:
    char _buf[BUFF_SIZE];
};

//бс=============================================================================================бс
//   HNetIoWsRecv.
//бс=============================================================================================бс
class HNetIoWsRecv : public HNetIo
{
public:
    HNetIoWsRecv(HNetSession* session);
    ~HNetIoWsRecv();

    bool Create();
    bool Initialize();
	bool IsValid(DWORD size);

    WSABUF* GetWsaBuf();
    bool Completed(DWORD bytes);

    void RecvWs();
    void RecvedWs(DWORD bytes);

private:
    USHORT _Preprocess(HNetWsPacket::Header* header);
    void _Pong();

private:
    char* _buf;

    DWORD _size;

    DWORD _begin;
    DWORD _end;
};


//бс=============================================================================================бс
//   job type.
//бс=============================================================================================бс
enum class JOB_TYPE : int
{
      ACCEPT
    , RECONNECT
    , DISCONNECT
    , RECV
    , SEND
};

//бс=============================================================================================бс
//   HNetIoJob.
//бс=============================================================================================бс
class HNetIoJob
{
public:
    virtual ~HNetIoJob() {}
    virtual void Excute() = 0;

    void* operator new(size_t size);
    void operator delete(void* p);
};

//бс=============================================================================================бс
//   HNetIoAcceptJob.
//бс=============================================================================================бс
class HNetIoAcceptJob : public HNetIoJob, public HNetIoAccept
{
public:
    HNetIoAcceptJob(HNetIoAccept* io);
    void Excute() override;
};

//бс=============================================================================================бс
//   HNetIoReconnectJob.
//бс=============================================================================================бс
class HNetIoReconnectJob : public HNetIoJob, public HNetIoReconnect
{
public:
    HNetIoReconnectJob(HNetIoReconnect* io);
    void Excute() override;
};

//бс=============================================================================================бс
//   HNetIoDisconnectJob.
//бс=============================================================================================бс
class HNetIoDisconnectJob : public HNetIoJob, public HNetIoDisconnect
{
public:
    HNetIoDisconnectJob(HNetIoDisconnect* io);
    void Excute() override;
};

//бс=============================================================================================бс
//   HNetIoRecvJob.
//бс=============================================================================================бс
class HNetIoRecvJob : public HNetIoJob, public HNetIoRecv
{
public:
    HNetIoRecvJob(HNetIoRecv* io);
    void Excute() override;

private:
    DWORD _byte;
};

//бс=============================================================================================бс
//   HNetIoSendJob.
//бс=============================================================================================бс
class HNetIoSendJob : public HNetIoJob, public HNetIoSend
{
public:
    HNetIoSendJob(HNetIoSend* io);
    void Excute() override;
};

#define HNET_IO\
    public HNetIoListen,\
    public HNetIoConnect,\
    public HNetIoReconnect,\
    public HNetIoDisconnect,\
    public HNetIoSend,\
    public HNetIoAccept,\
    public HNetIoRecv,\
    public HNetIoWsAccept,\
    public HNetIoWsRecv

#define FRIEND_HNET_IO\
    friend HNetIoListen;\
    friend HNetIoConnect;\
    friend HNetIoReconnect;\
    friend HNetIoDisconnect;\
    friend HNetIoSend;\
    friend HNetIoAccept;\
    friend HNetIoRecv;\
    friend HNetIoWsAccept;\
    friend HNetIoWsRecv;