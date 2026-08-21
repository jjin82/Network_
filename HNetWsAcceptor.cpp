#include "HNetWsAcceptor.h"

#ifdef HNETWORK_FULL_SOURCE

void HNetWsAcceptor::OnMessage(NetId netId, const HNetWsPacket& packet)
{
    OnMessage(netId, NULL);
};

void HNetWsAcceptor::OnMessage(NetId netId, byte const* byte)
{

}

void HNetWsAcceptor::OnRegMessage()
{

}

HNetWsAcceptor::HNetWsAcceptor()
{
	
}

HNetWsAcceptor::~HNetWsAcceptor()
{
        
}

bool HNetWsAcceptor::Listen()
{
    return Listen(HNET_DEFAULT_PORT);
}

bool HNetWsAcceptor::Listen(USHORT port)
{
    return Listen(NULL, port);
}

bool HNetWsAcceptor::Listen(const wchar_t* host, USHORT port)
{
    HNetAcceptOption option;
    option.Host(host);
    option.Port(port);
    option.Name(L"HNet WsAcceptor");

    return Listen(option);
}

bool HNetWsAcceptor::Listen(HNetAcceptOption& option)
{
    if (true == _Session()->IsValid())
        return true;

    if (false == HNet::_Create(option._id, option._name, option._sessionCount, option._threadCount))
        return false;

    if (false == _Session()->Listen(option._host, option._port))
    {
        LOG_CRITICAL_SYSTEM(L"listen failed. host= %s, port= %d", option._host, option._port);
        return false;
    }

    for (int i = 0; i < option._sessionCount; ++i)
    {
        HNetSession* session = _CreateSession();
        if (NULL == session)
            return false;

        if (false == session->AcceptWs(_Session()))
            return false;
    }

    LOG_INFO_SYSTEM(L"¡ß WsAcceptor. ip= %s, port= %d, id= %d, name= %s, session count= %d, thread= %d",
        _Session()->GetIp(), 
        _Session()->GetPort(), 
        option._id, 
        option._name, 
        option._sessionCount, 
        option._threadCount);

    return true;
}

void HNetWsAcceptor::Disconnect(NetId netId)
{
	HNetSession* session = _FindSession(netId);
	if (NULL == session)
		return;

    session->Disconnect();
}

bool HNetWsAcceptor::_Connected(HNetSession* session)
{
	if (false == HNet::_ConnectedWs(session))
        return false;

	OnConnect(session->GetNetId());
    
    return true;
}

bool HNetWsAcceptor::_Disconnected(HNetSession* session)
{
	if (false == HNet::_Disconnected(session))
		return false;

	OnDisconnect(session->GetNetId(), session->DisconnectReason());

    session->AcceptWs(_Session());

	return true;
}

void HNetWsAcceptor::_Message(HNetSession* session, const HNetWsPacket& packet)
{
    if (HNetFunctor::_CallFunction(this, session->GetNetId(), packet))
        return;

    OnMessage(session->GetNetId(), (const HNetWsPacketEx&)packet);
}

void HNetWsAcceptor::_RegMessage()
{
    OnRegMessage();
}

#endif