#include "HNetSession.h"
#include "HNet.h"

HNet::HNet()
    : _create(false)
    , _id(0)
    , _rootSession(this, 0)
    , _connectedSessions(NULL)
{
    _name[0] = '\0';

    WSAStart();
}

HNet::~HNet()
{
    HNetIocp::_Destroy();

    // 메모리 반환.
    for (auto& session : _sessions)
        delete session;
    
    // 자료 비우기.
    _sessions.clear();
    _connectedSessions.clear();

    WSAEnd();
}

bool HNet::_Create(int id, const wchar_t* name, USHORT sessionCount, USHORT threadCount)
{
    if (true == _create)
        return true;

    if (false == HNetIocp::_Create(threadCount, IsSingleThread()))
        return false;

    sessionCount = min(60000, sessionCount);

    _sessions.reserve(sessionCount);
    if (sessionCount != _sessions.capacity())
        return false;

    _connectedSessions.reserve(sessionCount);
    if (sessionCount != _connectedSessions.capacity())
        return false;

    for(size_t i = 0; i < sessionCount; ++i)
        _connectedSessions.push_back(NULL);

    _id = id;
    HAPI::StringCopy(_name, name);

    _RegMessage();

    _create = true;

    return true;
}

bool HNet::_Connected(HNetSession* session)
{
    if (false == session->Initialize())
    {
        session->Disconnect(L"Session initialization has failed.");
        return false;
    }

    if (false == HNetIocp::_Associate(session))
    {
        session->Disconnect(L"Failed to register the session with IOCP.");
        return false;
    }

    if (false == _AddSession(session))
    {
        session->Disconnect(L"Failed to register the session.");
        return false;
    }

    return true;
 }

bool HNet::_Reconnected(HNetSession* session)
{
    return false;
}

bool HNet::_ConnectedWs(HNetSession* session)
{
    if (false == session->InitializeWs())
    {
        session->Disconnect(L"Session initialization has failed.(ws)");
        return false;
    }

    if (false == HNetIocp::_Associate(session))
    {
        session->Disconnect(L"Failed to register the session with IOCP.(ws)");
        return false;
    }

    if (false == _AddSession(session))
    {
        session->Disconnect(L"Failed to register the session.(ws)");
        return false;
    }

    return true;
}

bool HNet::_Disconnected(HNetSession* session)
{
	if (false == _DelSession(session))
		return false;

	return true;
}

void HNet::_Shutdown()
{
    for (auto& session : _connectedSessions)
    {
        if (NULL == session)
            continue;

        session->Disconnect(L"The server has been shut down.");
    }
}

void HNet::_Message(HNetSession* session, const HNetPacket& packet)
{

}

void HNet::_Message(HNetSession* session, const HNetWsPacket& packet)
{

}

void HNet::_Encrypt(WsaSend* wsaSend)
{
    if (false == wsaSend->NotEncrypt())
        return;

    char*  buf   = wsaSend->Body();
    USHORT lenth = wsaSend->BodyLenth();

    for (int i = 0; i < lenth; ++i)
    {
        buf[i] ^= 0xEE;
    }
}

void HNet::_Decrypt(USHORT type, char* buf, SHORT lenth)
{
    for (int i = 0; i < lenth; ++i)
    {
        buf[i] ^= 0xEE;
    }
}

void HNet::_AddIoJob(HNetIoJob* io)
{
    Lock();
    {
        _ioJobQueue.push(io);
    }
    Unlock();
}

void HNet::_AddReference(int netId)
{
    HNetSession* session = _FindSession(netId);
    if (NULL == session) return;

    session->Reference();
}

void HNet::_ReleaseReference(int netId)
{
    HNetSession* session = _FindSession(netId);
    if (NULL == session) return;

    session->Release();
}

void HNet::_Send(WsaSend* wsaSend)
{
    _Encrypt(wsaSend);

    _Session()->Send(wsaSend);
}

void HNet::_Send(NetId netId, WsaSend* wsaSend)
{
    HNetSession* session = _FindSession(netId);
    if (NULL == session) return;
    
    _Encrypt(wsaSend);

    session->Send(wsaSend);
}

void HNet::_SendAll(WsaSend* wsaSend)
{
    _Encrypt(wsaSend);
  
    for (auto& session : _connectedSessions)
    {
        if (NULL == session) continue;

        session->Send(wsaSend);
    }
}

HNetSession* HNet::_Session()
{
    return &_rootSession;
}

HNetSession* HNet::_ListenSession()
{
    return &_rootSession;
}

HNetSession* HNet::_CreateSession()
{
    if (_sessions.size() >= _connectedSessions.capacity())
        return NULL;

    HNetSession* session = new(std::nothrow) HNetSession(this, (USHORT)_sessions.size());
    if (NULL == session) return NULL;

    if (false == session->Create())
    {
        delete session;
        return NULL;
    }

    _sessions.push_back(session);

    return session;
}

HNetSession* HNet::_FindSession(NetId netId)
{
	size_t index = (netId & 0xFFFF);
	if (index >= _sessions.size())
		return NULL;

	if (netId != _sessions[index]->GetNetId())
		return NULL;

	return _sessions[index];
}

bool HNet::_AddSession(HNetSession* session)
{
    SCOPED_SLOCK(*this);

    if (NULL != _connectedSessions[session->Id()])
        return false;

    _connectedSessions[session->Id()] = session;

    return true;
}

bool HNet::_DelSession(HNetSession* session)
{
    SCOPED_SLOCK(*this);
    
    if (NULL == _connectedSessions[session->Id()])
        return false;

    _connectedSessions[session->Id()] = NULL;

    return true;
}

bool HNet::IsSingleThread()
{
    return false;
}

void HNet::Update()
{
    Lock();
    {
        while (0 < _ioJobQueue.size())
        {
            HNetIoJob* job = _ioJobQueue.front();
            job->Excute();
            delete job;

            _ioJobQueue.pop();
        }
    }
    Unlock();
}

int HNet::GetId()
{
    return _id;
}

wchar_t* HNet::GetName()
{
    return _name;
}

HNetHost* HNet::GetHost(NetId netId)
{
    HNetSession* session = _FindSession(netId);
    if (NULL == session)
        return NULL;

    static thread_local HNetHost Host;
    session->GetHost(Host);

    return &Host;
}