#pragma once

#include "HNetCommon.h"

class HNetIocp
{
protected:
    HNetIocp();
    ~HNetIocp();

    bool _Create(USHORT threadCount, bool single);
    void _Destroy();

    bool _Associate(HNetSession* session);
    bool _PostIo(HNetIo* io);

private:
    HANDLE  _iocp;
    HANDLE* _thread;
    USHORT  _threadCount;
};
