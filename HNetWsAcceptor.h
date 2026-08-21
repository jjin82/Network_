#pragma once

#include "HNet.h"

#ifdef HNETWORK_FULL_SOURCE

//бс=============================================================================================бс
//   HNetWsAcceptor: web socket
//бс=============================================================================================бс
class HNetWsAcceptor : public HNet
{
	virtual void OnConnect(NetId netId) = 0;
	virtual void OnDisconnect(NetId netId, const wchar_t* reason) = 0;
	virtual void OnMessage(NetId netId, const HNetWsPacket& packet);
    virtual void OnMessage(NetId netId, byte const* byte);
    virtual void OnRegMessage();

protected:
	HNetWsAcceptor();
    ~HNetWsAcceptor();
    
    T2(T, P) void RegMessage(void (T::*func)(NetId, const P&)) { HNetFunctor::_RegFunction(func); }

public:
    bool Listen();
    bool Listen(USHORT port);
    bool Listen(const wchar_t* host, USHORT port);
    bool Listen(HNetAcceptOption& option);
	void Disconnect(NetId netId);

    T1(P) void Send(NetId netId, HNET::NewWsPacketT<P>& packet) { _Send(netId, packet); }
    T1(P) void SendAll(HNET::NewWsPacketT<P>& packet)           { _SendAll(packet); }

private:
	bool _Connected(HNetSession* session) override;
	bool _Disconnected(HNetSession* session) override;
	void _Message(HNetSession* session, const HNetWsPacket& packet) override;
    void _RegMessage() override;
};

namespace HNET
{
    typedef HNetWsAcceptor WsAcceptor;
}

#endif