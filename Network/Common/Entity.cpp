#pragma once

#include "Entity.h"

unsigned long GetEntityKey()
{
	static unsigned int s_key = GetTickCount();

	unsigned int key = InterlockedIncrement(&s_key);
	if (0 == key)
		key = InterlockedIncrement(&s_key);

	return key;
}
