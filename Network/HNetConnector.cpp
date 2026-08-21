#include "HNetConnector.h"

void HNetConnector::OnReconnected()
{
    LOG_INFO_SYSTEM(L"[%s] 'OnReconnected' function call", HNet::GetName());
}

void HNetConnector::OnMessage(const HNetPacketEx& packet)
{
    OnMessage((HNetStreamEx&)packet);
}

void HNetConnector::OnMessage(const HNetStream& stream)
{
    LOG_INFO_SYSTEM(L"[%s] 'OnMessage' function call", HNet::GetName());
}

void HNetConnector::OnRegMessage()
{
    LOG_INFO_SYSTEM(L"[%s] 'OnRegMessage' function call", HNet::GetName());
}

HNetConnector::HNetConnector()
    : _reconnect(true)
{

}

HNetConnector::~HNetConnector()
{

}

bool HNetConnector::Connect()
{
    return Connect(HNET_DEFAULT_PORT);
}

bool HNetConnector::Connect(USHORT port)
{
    return Connect(NULL, port);
}

bool HNetConnector::Connect(const wchar_t* host)
{
    return Connect(host, HNET_DEFAULT_PORT);
}

bool HNetConnector::Connect(const wchar_t* host, USHORT port)
{
    HNetConnectOption option;
    option.Host(host);
    option.Port(port);
    option.Name(L"HNet Connector");

    return Connect(option);
}

bool HNetConnector::Connect(HNetConnectOption& option)
{
    if (true == _Session()->IsValid())
        return true;

    if (false == HNet::_Create(option._id, option._name, 1, 1))
        return false;

    if (false == _Session()->Create())
        return false;

    if (false == _Session()->Connect(option._host, option._port))
        return false;

    _reconnect = true;

    LOG_INFO_SYSTEM(L"◆ Connector. ip= %s, port= %d, id= %d, name= %s",
        _Session()->GetIp(), 
        _Session()->GetPort(), 
        option._id, 
        option._name);

    return true;
}

void HNetConnector::Disconnect()
{
    // 접속 종료를 요청하면 재접속을 하지 않는다.
    _reconnect = false;

	_Session()->Disconnect();
}

bool HNetConnector::_Connected(HNetSession* session)
{
	if (false == HNet::_Connected(session))
        return false;
    
	OnConnected();

    return true;
}

bool HNetConnector::_Reconnected(HNetSession* session)
{
    if (false == HNet::_Connected(session))
        return false;

    OnReconnected();

    return true;
}

bool HNetConnector::_Disconnected(HNetSession* session)
{
	if (false == HNet::_Disconnected(session))
		return false;

	OnDisconnected(session->DisconnectReason());

    if (_reconnect)
        session->TryReconnect();
    else
        session->Close();

    return true;
}

void HNetConnector::_Message(HNetSession* session, const HNetPacket& packet)
{
    if (HNetFunctor::_CallFunction(this, packet))
        return;

	OnMessage((const HNetPacketEx&)packet);
}

void HNetConnector::_RegMessage()
{
	OnRegMessage();
}
