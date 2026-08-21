#pragma once
#pragma warning(disable:4995)

#include "HNetCommon.h"


extern Performance g_performance;

namespace HNET
{
    namespace PERFORMANCE
    {
        void NextRecvInfo(OUT char*& IOCount, OUT char*& count, OUT char*& maxCount, OUT char*& bytes, OUT char*& maxBytes)
        {
            g_performance.NextRecvInfo(IOCount, count, maxCount, bytes, maxBytes);
        }

        void NextSendInfo(OUT char*& IOCount, OUT char*& count, OUT char*& maxCount, OUT char*& bytes, OUT char*& maxBytes)
        {
            g_performance.NextSendInfo(IOCount, count, maxCount, bytes, maxBytes);
        }
    }
}

