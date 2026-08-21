#include "HNetAcceptor.h"

void HNetAcceptor::OnMessage(NetId netId, const HNetPacketEx& packet)
{
    OnMessage(netId, (HNetStreamEx&)packet);
};

void HNetAcceptor::OnMessage(NetId netId, const HNetStream& stream)
{ 
    
}

void HNetAcceptor::OnRegMessage()
{

}

HNetAcceptor::HNetAcceptor()
{
	
}

HNetAcceptor::~HNetAcceptor()
{
        
}

bool HNetAcceptor::Listen()
{
    return Listen(HNET_DEFAULT_PORT);
}

bool HNetAcceptor::Listen(USHORT port)
{
    return Listen(NULL, port);
}

bool HNetAcceptor::Listen(const wchar_t* host)
{
    return Listen(host, HNET_DEFAULT_PORT);
}

bool HNetAcceptor::Listen(const wchar_t* host, USHORT port)
{
    HNetAcceptOption option;
    option.Host(host);
    option.Port(port);
    option.Name(L"HNet Acceptor");

    return Listen(option);
}

bool HNetAcceptor::Listen(HNetAcceptOption& option)
{
    if (true == _ListenSession()->IsValid())
        return true;

    if (false == HNet::_Create(option._id, option._name, option._sessionCount, option._threadCount))
        return false;

    if (false == _ListenSession()->Listen(option._host, option._port))
    {
        LOG_CRITICAL_SYSTEM(L"listen failed. host= %s, port= %d", option._host, option._port);
        return false;
    }

    for (USHORT i = 0; i < option._sessionCount; ++i)
    {
        HNetSession* session = _CreateSession();
        if (NULL == session)
            return false;

        if (false == session->Accept(_ListenSession()))
            return false;
    }

    LOG_INFO_SYSTEM(L"¡ß Accepter. ip(%s)= %s, port= %d, id= %d, name= %s, session= %d, thread= %d",
        _ListenSession()->GetAddressFamilyString(), 
        _ListenSession()->GetIp(), 
        _ListenSession()->GetPort(), 
        option._id, 
        option._name, 
        option._sessionCount, 
        option._threadCount);

    return true;
}

void HNetAcceptor::Disconnect(NetId netId)
{
	HNetSession* session = _FindSession(netId);
	if (NULL == session) return;

	session->Disconnect();
}

void HNetAcceptor::Shutdown()
{
    HNet::_Shutdown();
}

void HNetAcceptor::AddReference(int netId)
{
    _AddReference(netId);
}

void HNetAcceptor::ReleaseReference(int netId)
{
    _ReleaseReference(netId);
}

bool HNetAcceptor::_Connected(HNetSession* session)
{
	if (false == HNet::_Connected(session))
        return false;

	OnConnected(session->NewNetId());
    
    return true;
}

bool HNetAcceptor::_Disconnected(HNetSession* session)
{
	if (false == HNet::_Disconnected(session))
		return false;

	OnDisconnected(session->GetNetId(), session->DisconnectReason());

    session->Close();

	return session->Accept(_ListenSession());
}

void HNetAcceptor::_Message(HNetSession* session, const HNetPacket& packet)
{
    if (HNetFunctor::_CallFunction(this, session->GetNetId(), packet))
        return;

    OnMessage(session->GetNetId(), (const HNetPacketEx&)packet);
}

void HNetAcceptor::_RegMessage()
{
    OnRegMessage();
}