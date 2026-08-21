#include "Common.h"

namespace HNET
{
    namespace LIB
    {
        Flag::Flag(int count)
            : _maxCount(count)
		{ 
            _flag = new(std::nothrow) bool[_maxCount];
            if (NULL == _flag)
            {
                MSG_BOX_DETAIL(L"error");
                return;
            }
            
            ZeroMemory(_flag, sizeof(bool) * _maxCount); 
        }

	    void Flag::Enable(const DWORD flag)
	    {
		    if (_maxCount > flag)
			    _flag[flag] = true;
	    }

	    void Flag::Disable(const DWORD flag)
	    {
		    if (_maxCount > flag)
			    _flag[flag] = false;
	    }

	    bool Flag::IsFlag(const DWORD flag)
	    {
		    if (_maxCount > flag)
			    return _flag[flag];

		    return false;
	    }
    }
}