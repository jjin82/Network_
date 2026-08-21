#pragma once

#include "HNet.h"

//бс=============================================================================================бс
//   Acceptor.
//бс=============================================================================================бс
class HNetAcceptor : public HNet
{
    virtual void OnConnected(NetId netId) = 0;
    virtual void OnDisconnected(NetId netId, const wchar_t* reason) = 0;
    virtual void OnMessage(NetId netId, const HNetPacketEx& packet);
    virtual void OnMessage(NetId netId, const HNetStream& stream);
    virtual void OnRegMessage();

protected:
    HNetAcceptor();
    virtual ~HNetAcceptor();

    T2(T, P) void RegMessage(void (T::*func)(NetId, const P&)) { HNetFunctor::_RegFunction(func); }

public:
    bool Listen();
    bool Listen(USHORT port);
    bool Listen(const wchar_t* host);
    bool Listen(const wchar_t* host, USHORT port);
    bool Listen(HNetAcceptOption& option);
    void Disconnect(NetId netId);
    void Shutdown();

    void AddReference(int netId);
    void ReleaseReference(int netId);

    T1(P) void Send(NetId netId, HNET::NewPacketT<P>& packet) { _Send(netId, packet); }
    T1(P) void SendAll(HNET::NewPacketT<P>& packet)           { _SendAll(packet); }

    void Send(NetId netId, HNET::Packet& packet)  { }
    void SendAll(HNET::Packet& packet)            { }

private:
    bool _Connected(HNetSession* session) override;
    bool _Disconnected(HNetSession* session) override;
    void _Message(HNetSession* session, const HNetPacket& packet) override;
    void _RegMessage() override;
};

namespace HNET
{
    typedef HNetAcceptor Acceptor;
}