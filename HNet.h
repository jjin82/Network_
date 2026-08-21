#pragma once

#include "HNetSession.h"
#include "HNetOption.h"
#include "HNetIocp.h"
#include "HNetFunctor.h"
#include <vector>
#include <queue>

typedef std::vector<HNetSession*> SESSIONS;
typedef std::queue<HNetIoJob*>    IO_JOB_QUEUE;

//бс=============================================================================================бс
//   HNet.
//бс=============================================================================================бс
class HNet 
    : HLIB::SpinLock
    , public HNetIocp
    , public HNetFunctor
{
    friend class HNetSession;

protected:
    HNet();
    virtual ~HNet();

    bool _Create(int id, const wchar_t* name, USHORT sessionCount, USHORT threadCount);
    
    virtual bool _Connected(HNetSession* session);
    virtual bool _Reconnected(HNetSession* session);
    virtual bool _ConnectedWs(HNetSession* session);
    virtual bool _Disconnected(HNetSession* session);
    virtual void _Shutdown();
    virtual void _Message(HNetSession* session, const HNetPacket& packet);
    virtual void _Message(HNetSession* session, const HNetWsPacket& packet);
    virtual void _RegMessage() = 0;
    virtual void _Encrypt(WsaSend* wsaSend);
    virtual void _Decrypt(USHORT type, char* buf, SHORT lenth);
    
    void _AddIoJob(HNetIoJob* io);
    void _AddReference(int netId);
    void _ReleaseReference(int netId);
    
    void _Send(WsaSend* wsaSend);
    void _Send(NetId netId, WsaSend* wsaSend);
    void _SendAll(WsaSend* wsaSend);

    HNetSession* _Session();
    HNetSession* _ListenSession();
    HNetSession* _CreateSession();
    HNetSession* _FindSession(NetId netId);
    bool _AddSession(HNetSession* session);
    bool _DelSession(HNetSession* session);

public:
    virtual bool IsSingleThread();
    void Update();

    int GetId();
    wchar_t* GetName();
    HNetHost* GetHost(NetId netId);

private:
    bool          _create;
    int           _id;                       // id.
    wchar_t       _name[HNET_NAME_LENTH];    // name.
    HNetSession   _rootSession;              // listen, connect session.
    SESSIONS      _sessions;                 // session list.
    SESSIONS      _connectedSessions;        // connected sessions.
    IO_JOB_QUEUE  _ioJobQueue;               // io job queue.
};
