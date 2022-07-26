#include "Engine/Core/JobSystem.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

// =================================================================================
// =================================================================================
JobState Job::GetState() const
{
	return m_state;
}


std::atomic_int Job::s_idCounter = 0;

// =================================================================================
// =================================================================================
JobSystem::JobSystem(const JobSystemConfig& theConfig)
    : m_theConfig(theConfig)
{
}

JobSystem::~JobSystem()
{
}

void JobSystem::Startup()
{
	DebuggerPrintf("[JobSystem] Starting up...\n");
	m_isQuitting = false;

	for (int i = 0; i < m_theConfig.m_workers; i++)
	{
		m_workers.push_back(new Worker(this));
	}
}

void JobSystem::BeginFrame()
{
}

void JobSystem::EndFrame()
{
}

void JobSystem::Shutdown()
{
	DebuggerPrintf("[JobSystem] Shutting down...\n");
	m_isQuitting = true;

	// clear queued jobs
	{ // mutex lock: m_queuedJobs
		std::lock_guard<std::mutex> guard(m_queuedJobsLock);

		for (auto& job : m_queuedJobs)
			delete job;
		m_queuedJobs.clear();
	}

	// wake up waiting worker
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_cond.notify_all();
	}

	// clean up workers
	for (auto& worker : m_workers)
	{
		delete worker;
	}
	m_workers.clear();

	// call finish on all finished jobs
	FinishUpJobs();
}

void JobSystem::FinishUpJobs(int count/* = -1*/)
{
	std::vector<Job*> jobs;
	{ // mutex lock: m_finishedJobs
		std::lock_guard<std::mutex> guard(m_finishedJobsLock);

		while (count)
		{
			if (m_finishedJobs.empty())
				break;

			Job* job = m_finishedJobs.front();
			m_finishedJobs.pop_front();
			if (job)
			{
				jobs.push_back(job);
				count--;
			}
		}
	}

	for (Job* job : jobs)
	{
		job->OnFinished();
		if (job->m_destroyAfterFinished)
			delete job;
	}
}

void JobSystem::FinishUpJobsOfType(int type,int count/* = -1*/)
{
	std::vector<Job*> jobs;
	{ // mutex lock: m_finishedJobs
		std::lock_guard<std::mutex> guard(m_finishedJobsLock);

		// loop through m_finishedJobs
		for (auto& job : m_finishedJobs)
		{
			if (!count)
				break;

			if (job && job->m_type == type)
			{
				// save and count
				count--;
				jobs.push_back(job);
				job = nullptr;
			}
		}

		if (m_finishedJobs.size())
			while (!m_finishedJobs.empty() && !m_finishedJobs.front())
				m_finishedJobs.pop_front();
	}

	for (Job* job : jobs)
	{
		job->OnFinished();
		if (job->m_destroyAfterFinished)
			delete job;
	}
}

void JobSystem::QueueJob(Job* job)
{
	{ // mutex lock: m_queuedJobs
		std::lock_guard<std::mutex> guard(m_queuedJobsLock);

		DebuggerPrintf("[JobSystem] Queuing job-%d\n", job->m_id);
		job->m_state = JobState::QUEUED;
		m_queuedJobs.push_back(job);

		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_cond.notify_one();
		}
	}
}

Job* JobSystem::GrabQueuedJob()
{
	Job* job = nullptr;
	{ // mutex lock: m_queuedJobs
		std::lock_guard<std::mutex> guard(m_queuedJobsLock);

		if (!m_queuedJobs.empty())
		{
			job = m_queuedJobs.front();
			m_queuedJobs.pop_front();
		}
	}

	if (job)
	{
		std::lock_guard<std::mutex> guardExecuting(m_executingJobsLock);
		m_executingJobs.push_back(job);
	}
	return job;
}

void JobSystem::ReturnFinishedJob(Job* job)
{
	{
		std::lock_guard<std::mutex> guardExecuting(m_executingJobsLock);

		for (auto ite = m_executingJobs.begin(); ite != m_executingJobs.end(); ite++)
		{
			if (*ite == job)
			{
				m_executingJobs.erase(ite);
				break;
			}
		}
	}

	{ // mutex lock: m_finishedJobs
		std::lock_guard<std::mutex> guard(m_finishedJobsLock);

		job->m_state = JobState::FINISHED;
		m_finishedJobs.push_back(job);
	}
}

Job::Job(int type)
	: m_id(s_idCounter++)
	, m_type(type)
{
}

Job::~Job()
{
}


Worker::Worker(JobSystem* system)
	: m_system(system)
	, m_id(s_idCounter++)
{
	m_thread = new std::thread([this]() { RunWorkerThread(); });
}

Worker::~Worker()
{
	if (m_thread)
	{
		m_thread->join();
		delete m_thread;
		m_thread = nullptr;
	}
}

void Worker::RunWorkerThread()
{
	DebuggerPrintf("[Worker-%d] Starting thread...\n", m_id);

	while (true)
	{
		{
			std::unique_lock<std::mutex> lock(m_system->m_mutex);
			m_system->m_cond.wait(lock);
		}

		if (m_system->m_isQuitting)
		{
			DebuggerPrintf("[Worker-%d] Quitting thread...\n", m_id);
			break;
		}

		bool runned = false;
		while (Job* job = m_system->GrabQueuedJob())
		{
			runned = true;
			DebuggerPrintf("[Worker-%d] Working on job-%d\n", m_id, job->m_id);
			job->m_state = JobState::EXECUTING;
			m_workingJob = job;
			job->Execute();
			job->m_state = JobState::FINISHED;
			m_workingJob = nullptr;
			m_system->ReturnFinishedJob(job);
			DebuggerPrintf("[Worker-%d] Finished job-%d\n", m_id, job->m_id);
		}

		if (m_system->m_isQuitting)
		{
			DebuggerPrintf("[Worker-%d] Quitting thread...\n", m_id);
			break;
		}

		if (!runned)
		{
			// false awake
			DebuggerPrintf("[Worker-%d] false awake\n", m_id);
		}
	}

}

std::atomic_int Worker::s_idCounter = 0;
