#include "Common.h"
#include "Job.h"
#include <queue>
#include <utility> // pair를 위한 라이브러리


//■=============================================================================================■
//   Job Priority.
//■=============================================================================================■
struct JobPriority
{
public:
    JobPriority(DWORD millisecond, std::function<void()> func)
        : _executeTime(timeGetTime() + millisecond)
        , _func(func)
    {

    }

    // 수행 시간이 작은것이 Top 을 유지 한다
    bool operator<(const JobPriority job) const 
    {
        return _executeTime > job._executeTime;
    }

public:
    DWORD                   _executeTime;
    std::function<void()>   _func;
};


//■=============================================================================================■
//   Job Queue.
//■=============================================================================================■
typedef std::queue<std::function<void()>> JOB_QUEUE;
typedef std::priority_queue<JobPriority>  JOB_QUEUE_TIMER;


class JobQueue : public HNET::LIB::SpinLock
{
public:
    JobQueue(bool thread = true)
        : _active(true)
        , _jobID(0xFFFFFFFF)
        , _thread(NULL)
    {
        static unsigned int count = 0;

        _active = true;
        _jobID  = InterlockedIncrement(&count) - 1;

        if (thread)
        {
            unsigned int threadId;
            _thread = (HANDLE)_beginthreadex(NULL, 0, _Worker, this, 0, &threadId);
            if (NULL == _thread)
            {
                _active = false;
                LOG_LAST_ERROR_DETAIL_SYSTEM();
                return;
            }
        }

        // 메모리풀을 사용하므로 레퍼 증가.
        HNET::SINGLETON::MemoryReference();
    }

    ~JobQueue()
    {
        if (false == _active)
            return;

        _active = false;

        if (NULL != _thread)
        {
            WaitForSingleObject(_thread, INFINITE);
            _thread = NULL;
        }

        // 메모리풀을 사용했으므로 레퍼 감소.
        HNET::SINGLETON::MemoryRelease(); 
    }

    bool Post(std::function<void()> f, DWORD delay)
    {
        if (false == _active)
        {
            LOG_CRITICAL_SYSTEM(L"failed to initialize job queue.");
            return false;
        }

        if (NULL == f)
            return false;

        Lock();
        {
            (0 == delay) ? _queue.push(f) : _queueTimer.emplace(delay, f);
        }
        Unlock();

        return true;
    }

    void Execute()
    {
        if (false == _active)
            return;

        Lock();
        {
            while (false == _queue.empty())
            {
                _queue.front()();
                _queue.pop();
            }

            while (false == _queueTimer.empty())
            {
                JobPriority jobPrio = _queueTimer.top();
                if (jobPrio._executeTime > timeGetTime())
                    break;

                // 타이머 job에서 제거.
                _queueTimer.pop();

                // 수행.
                jobPrio._func();
            }
        }
        Unlock();
    }

    static unsigned WINAPI _Worker(void* p)
    {
        JobQueue* jobQueue = (JobQueue*)p;
        
        tls_jobId = jobQueue->_jobID;

        while (jobQueue->_active)
        {
            jobQueue->Execute();

            // 초당 100회 처리.
            HNET::API::Sleep(10);
        }

        return 0;
    }

private:
    bool            _active;
    int             _jobID;
    HANDLE          _thread;
    JOB_QUEUE       _queue;
    JOB_QUEUE_TIMER _queueTimer;
};

thread_local unsigned int tls_jobId = 0xFFFFFFFF;


//■=============================================================================================■
//   Job.
//■=============================================================================================■
Job::Job()
{
    _count  = HNET::API::ProcessorCount();
    _queues = new JobQueue[_count];
    _seqId  = 0;

    LOG_INFO_SYSTEM(L"■ job thread count( %d ) ■", _count);
}

Job::~Job()
{
    if (NULL == _queues)
        return;

    delete[] _queues;
    _queues = NULL;
}

JobQueue* Job::GetQueue(unsigned int id)
{
    return &_queues[_SafeId(id)];
}

void Job::Post(unsigned int id, std::function<void()> f, DWORD delay)
{
    _queues[_SafeId(id)].Post(f, delay);
}

void Job::Post(std::function<void()> f, DWORD delay)
{
    if (0xFFFFFFFF == tls_jobId)
    {
        GetQueue(++_seqId)->Post(f, delay);
    }
    else
    {
        GetQueue(tls_jobId)->Post(f, delay);
    }
}

void Job::Execute()
{
    for(unsigned int i = 0; i <= _count; ++i)
    {
        _queues[i].Execute();
    }
}

unsigned int Job::_SafeId(unsigned int id)
{
    // 배열 범위면 그냥 쓰고 아니면 나머지 연산으로 범위를 만들어줌.
    return (id < _count) ? id : (id % _count);
}