#include "Engine/Core/Stopwatch.hpp"

#include "Engine/Core/Clock.hpp"

Stopwatch::Stopwatch() : Stopwatch(&Clock::GetSystemClock())
{
}

Stopwatch::Stopwatch(double duration) : Stopwatch(&Clock::GetSystemClock(), duration)
{
}

Stopwatch::Stopwatch(const Clock* clock, double duration /*= 0*/)
	: m_clock(clock)
{
	if (duration)
	{
		Start(duration);
	}
}

void Stopwatch::Start(double duration)
{
	m_startTime = m_clock->GetTotalTime();
	m_duration = duration;
}

void Stopwatch::Start(const Clock* clock, double duration)
{
	m_clock = clock;
	Start(duration);
}

void Stopwatch::Restart()
{
	m_startTime = m_clock->GetTotalTime();
}

void Stopwatch::Stop()
{
	m_duration = 0;
}

void Stopwatch::Pause()
{
	if (!IsPaused())
	{
		m_startTime = -GetElapsedTime();
	}
}

void Stopwatch::Resume()
{
	if (IsPaused())
	{
		m_startTime += m_clock->GetTotalTime();
	}
}

double Stopwatch::GetElapsedTime() const
{
	if (IsStopped())
	{
		return 0;
	}

	if (IsPaused())
	{
		return -m_startTime;
	}

	return m_clock->GetTotalTime() - m_startTime;
}

float Stopwatch::GetElapsedFraction() const
{
	return (float)(GetElapsedTime() / m_duration);
}

bool Stopwatch::IsStopped() const
{
	return m_duration == 0;
}

bool Stopwatch::IsPaused() const
{
	return m_startTime < 0;
}

bool Stopwatch::HasDurationElapsed() const
{
	return GetElapsedTime() >= m_duration;
}

bool Stopwatch::CheckDurationElapsedAndDecrement()
{
	if (IsStopped())
	{
		return false;
	}

	if (!HasDurationElapsed())
	{
		return false;
	}

	m_startTime += m_duration;
	return true;
}

