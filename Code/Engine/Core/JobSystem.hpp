#pragma once

#include "JobSystemConfig.hpp"

#include <vector>
#include <deque>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>


// =================================================================================
// =================================================================================
class Job;
class Worker;
class JobSystem;


// =================================================================================

enum class JobState
{
    UNBOUND,
    QUEUED,
    EXECUTING,
    FINISHED,
};


// =================================================================================
class Job
{
    friend class Worker;
    friend class JobSystem;

public:
    Job(int type = 0);
    virtual ~Job();

    JobState GetState() const;

private:
    virtual void Execute() = 0;
    virtual void OnFinished() = 0;

private:
	static std::atomic_int           s_idCounter;
    std::atomic<JobState>            m_state = JobState::UNBOUND;

protected:
	const int                        m_id = 0;
	bool                             m_destroyAfterFinished = true; // Set this to false to prevent the JobSystem from deleting it after finished

public:
    const int                        m_type;
};


// =================================================================================
class Worker
{

public:
    Worker(JobSystem* system);
    ~Worker();

private:
    void RunWorkerThread();

private:
	static std::atomic_int           s_idCounter;
    const int                        m_id = 0;
    JobSystem*                       m_system = nullptr;
    std::thread*                     m_thread = nullptr;

    std::atomic<Job*>                m_workingJob = nullptr;
    
};


// =================================================================================
class JobSystem
{
    friend class Worker;

public:
    JobSystem(const JobSystemConfig& theConfig);
    ~JobSystem();

    void Startup();
    void BeginFrame();
    void EndFrame();
    void Shutdown();

    void FinishUpJobs(int count = -1);
    void FinishUpJobsOfType(int type, int count = -1);
    void QueueJob(Job* job);

protected:
    Job* GrabQueuedJob();
    void ReturnFinishedJob(Job* job);

protected:
    JobSystemConfig m_theConfig;

private:
	std::vector<Worker*>       m_workers;
	std::mutex                 m_mutex;
	std::condition_variable    m_cond;
    std::atomic_bool           m_isQuitting = true;

    std::mutex                 m_queuedJobsLock;
	std::deque<Job*>           m_queuedJobs;
	std::mutex                 m_executingJobsLock;
	std::vector<Job*>          m_executingJobs;
    std::mutex                 m_finishedJobsLock;
    std::deque<Job*>           m_finishedJobs;
};

