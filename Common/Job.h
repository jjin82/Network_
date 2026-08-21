#pragma once

#include "Memory.h"
#include "RefCount.h"
#include <functional>

class JobQueue;

class Job
{
public:
    Job();
    ~Job();
    
    JobQueue* GetQueue(unsigned int id);

    void Post(unsigned int id, std::function<void()> f, DWORD delay = 0);
    void Post(std::function<void()> f, DWORD delay = 0);

    void Execute();

private:
    unsigned int _SafeId(unsigned int id);

private:
    unsigned int _count;
    JobQueue*    _queues;
    unsigned int _seqId;
};

extern thread_local unsigned int tls_jobId;