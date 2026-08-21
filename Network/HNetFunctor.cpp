#include "HNetFunctor.h"


HNetFunctor::HNetFunctor()
{
	for (int i = 0; i < 0xFFFF; ++i)
		_funcs[i] = NULL;
}

HNetFunctor::~HNetFunctor()
{
	for (int i = 0; i < 0xFFFF; ++i)
	{
		if (NULL == _funcs[i])
			continue;

		delete _funcs[i];
        _funcs[i] = NULL;
	}
}

bool HNetFunctor::_CallFunction(void* hnet, const HNetPacket& packet)
{
    if (0 == _funcs[packet.Type()])
        return false;
    
    _funcs[packet.Type()]->Message(hnet, packet);
    return true;
}

bool HNetFunctor::_CallFunction(void* hnet, NetId netId, const HNetPacket& packet)
{
    if (NULL == _funcs[packet.Type()])
        return false;
    
    _funcs[packet.Type()]->Message(hnet, netId, packet);
    return true;
}

bool HNetFunctor::_CallFunction(void* hnet, NetId netId, const HNetWsPacket& packet)
{
    if (NULL == _funcs[packet.Type()])
        return false;
    
    _funcs[packet.Type()]->Message(hnet, netId, packet);
    return true;
}