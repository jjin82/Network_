#pragma once

#include "HNetCommon.h"
#include "HNetIo.h"

typedef sockaddr_storage ADDRESS;

class HNet;

class HNetSession : public HLIB::RefCount, HNET_IO
{
    FRIEND_HNET_IO
public:
	HNetSession(HNet* hnet, USHORT index);
	virtual ~HNetSession();

    bool Create();
    bool Initialize();
    bool InitializeWs();

    bool Open();
    bool Open(ADDRESS_FAMILY af);
    void Close();
    void Cancel();
    
	bool SetAddress(const wchar_t* host, USHORT port);
    bool SetAddress(ADDRESS* address);
	sockaddr* GetAddress();
    int GetAddressLenth();
    ADDRESS_FAMILY GetAddressFamily();
    const wchar_t* GetAddressFamilyString();

	const wchar_t* GetIp();
	USHORT GetPort();
    void GetHost(OUT HNetHost& info);

    NetId GetNetId();
    NetId NewNetId();
    USHORT Id();

    bool IsValid();

private:
    bool _AddIoJob(JOB_TYPE type);
    bool _HostToAddr(const wchar_t* host, USHORT port);

    bool _OnListen();
    bool _OnConnect();
    void _OnTryReconnect();
    void _OnReconnect();
    void _OnReconnectJob();
    void _OnAcceptJob();
    void _OnSendJob();
    void _OnConnectWs();
    void _OnMessage(const HNetPacket& packet);
    void _OnMessageJob();
    void _OnMessage(const HNetWsPacket& packet);
    void _OnDisconnect();
    void _OnDisconnectJob();

    void OnRelease() override;

public:
    operator HANDLE() { return (HANDLE)_socket; }
    operator SOCKET() { return _socket; }

private:
    bool    _create;
    SOCKET  _socket;
    ADDRESS _address;
    NetId   _netId;
    HNet*   _hnet;
};

    