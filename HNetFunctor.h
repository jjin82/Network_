#pragma once
#include "HNetPacket.h"
#include <typeinfo>

//бс=============================================================================================бс
//   HNetFunc.
//бс=============================================================================================бс
class HNetFunc
{
public:
    virtual ~HNetFunc() {}
    virtual void Message(void*, const HNetPacket&) {}
    virtual void Message(void*, NetId, const HNetPacket&){}
    virtual void Message(void*, NetId, const HNetWsPacket&) {}
};


//бс=============================================================================================бс
//   HNetConnectorFunc.
//бс=============================================================================================бс
T2(T, P) class HNetConnectorFunc : public HNetFunc
{
public:
    HNetConnectorFunc(void (T::*func)(const P&))
    {
        _func = func;
    }
    
    ~HNetConnectorFunc() override
    {

    }

    void Message(void* hnet, const HNetPacket& packet) override
    {
        ((T*)hnet->*_func)((const P&)packet);
    }

private:
    void (T::*_func)(const P&);
};


//бс=============================================================================================бс
//   HNetAcceptorFunc.
//бс=============================================================================================бс
T2(T, P) class HNetAcceptorFunc : public HNetFunc
{
public:
    HNetAcceptorFunc(void (T::*func)(NetId, const P&))
    {
		_func = func;
    }

    ~HNetAcceptorFunc() override
    {

    }

    void Message(void* hnet, NetId netId, const HNetPacket& packet) override
    {
        ((T*)hnet->*_func)(netId, (const P&)packet);
    }

    void Message(void* hnet, NetId netId, const HNetWsPacket& packet) override
    {
        ((T*)hnet->*_func)(netId, (const P&)packet);
    }

private:
    void (T::*_func)(NetId, const P&);
};


//бс=============================================================================================бс
//   HNetFunctor.
//бс=============================================================================================бс
class HNetFunctor
{
    friend class HNet;
    friend class HNetAcceptor;
    friend class HNetWsAcceptor;
    friend class HNetConnector;

private:
	HNetFunctor();
	~HNetFunctor();

    bool _CallFunction(void* hnet, const HNetPacket& packet);
    bool _CallFunction(void* hnet, NetId netId, const HNetPacket& packet);
    bool _CallFunction(void* hnet, NetId netId, const HNetWsPacket& packet);

    T2(T, P) void _RegFunction(void (T::*func)(const P&));
    T2(T, P) void _RegFunction(void (T::*func)(NetId, const P&));

private:
    HNetFunc* _funcs[0xFFFF];
};


T2(T, P) void HNetFunctor::_RegFunction(void (T::*func)(const P&))
{
    P packet;
    if (HNetPacket::MAX_TYPE < packet.Type())
    {
        MSG_BOX(L"error", L"not use packet type= %s(%d), max type(%d).", HAPI::CharToWChar(typeid(packet).name()), packet.Type(), HNetPacket::MAX_TYPE);
        return;
    }

    if (_funcs[packet.Type()])
    {
        MSG_BOX(L"error", L"already registered packet= %s(%d).", HAPI::CharToWChar(typeid(packet).name()), packet.Type());
        return;
    }

    _funcs[packet.Type()] = new HNetConnectorFunc<T, P>(func);
}

T2(T, P) void HNetFunctor::_RegFunction(void (T::*func)(NetId, const P&))
{
    P packet;
    if (HNetPacket::MAX_TYPE < packet.Type())
    {
        MSG_BOX(L"error", L"not use packet type= %s(%d), max type(%d).", HAPI::CharToWChar(typeid(packet).name()), packet.Type(), HNetPacket::MAX_TYPE);
        return;
    }

    if (_funcs[packet.Type()])
    {
        MSG_BOX(L"error", L"already registered packet= %s(%d).", HAPI::CharToWChar(typeid(packet).name()), packet.Type());
        return;
    }

    _funcs[packet.Type()] = new HNetAcceptorFunc<T, P>(func);
}