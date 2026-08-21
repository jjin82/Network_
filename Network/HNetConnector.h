#pragma once

#include "HNet.h"

//бс=============================================================================================бс
//   Connector.
//бс=============================================================================================бс
class HNetConnector : public HNet
{
    virtual void OnConnected() = 0;
    virtual void OnDisconnected(const wchar_t* reason) = 0;
    virtual void OnReconnected();
    virtual void OnMessage(const HNetPacketEx& packet);
    virtual void OnMessage(const HNetStream& stream);
    virtual void OnRegMessage();

protected:
    HNetConnector();
    virtual ~HNetConnector();
    
    T2(T, P) void RegMessage(void (T::*func)(const P&)) { HNetFunctor::_RegFunction(func); }

public:
    bool Connect();
    bool Connect(USHORT port);
    bool Connect(const wchar_t* host);
    bool Connect(const wchar_t* host, USHORT port);
    bool Connect(HNetConnectOption& option);
    void Disconnect();

    T1(P) void Send(HNET::NewPacketT<P>& packet) { _Send(packet); }

private:
    bool _Connected(HNetSession* session) override;
    bool _Reconnected(HNetSession* session) override;
    bool _Disconnected(HNetSession* session) override;
    void _Message(HNetSession* session, const HNetPacket& packet) override;
    void _RegMessage() override;

private:
    bool _reconnect;
};

namespace HNET
{
    typedef HNetConnector Connector;
}